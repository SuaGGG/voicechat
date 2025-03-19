#include "audio_device.hpp"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <atomic>

namespace voicechat {

// 使用智能指针管理PortAudio资源
class PortAudioManager {
public:
    static PortAudioManager& getInstance() {
        static PortAudioManager instance;
        return instance;
    }
    
    ~PortAudioManager() {
        if (paInitialized) {
            Pa_Terminate();
            paInitialized = false;
            std::cout << "PortAudio 全局终止" << std::endl;
        }
    }
    
    bool initialize() {
        if (!paInitialized) {
            PaError err = Pa_Initialize();
            if (err != paNoError) {
                std::cerr << "PortAudio 初始化失败: " << Pa_GetErrorText(err) << std::endl;
                return false;
            }
            paInitialized = true;
            std::cout << "PortAudio 全局初始化成功" << std::endl;
        }
        return true;
    }
    
    void incrementCount() { deviceInstanceCount++; }
    void decrementCount() { deviceInstanceCount--; }
    int getCount() const { return deviceInstanceCount; }
    
private:
    PortAudioManager() = default;
    PortAudioManager(const PortAudioManager&) = delete;
    PortAudioManager& operator=(const PortAudioManager&) = delete;
    
    bool paInitialized = false;
    std::atomic<int> deviceInstanceCount{0};
};

// PortAudioDevice实现
PortAudioDevice::PortAudioDevice(bool isInput)
    : initialized_(false)
    , isInput_(isInput)
    , currentDeviceIndex_(-1)
    , sampleRate_(44100)
    , channels_(2)
    , volume_(1.0f)
    , callback_()
    , stream_(nullptr)
    , buffer_(BUFFER_SIZE)
{
    // 增加实例计数
    PortAudioManager::getInstance().incrementCount();
    
    // 初始化PortAudio
    if (!PortAudioManager::getInstance().initialize()) {
        return;
    }
    
    // 检查是否有可用设备
    int numDevices = Pa_GetDeviceCount();
    if (numDevices <= 0) {
        std::cerr << "没有找到音频设备" << std::endl;
        return;
    }
    
    // 检查默认设备是否可用
    PaDeviceIndex deviceIndex = isInput_ ? Pa_GetDefaultInputDevice() : Pa_GetDefaultOutputDevice();
    if (deviceIndex == paNoDevice) {
        std::cerr << "没有找到默认" << (isInput_ ? "输入" : "输出") << "设备" << std::endl;
        return;
    }
    
    std::cout << "找到 " << numDevices << " 个设备" << std::endl;
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(deviceIndex);
    if (deviceInfo) {
        std::cout << "默认" << (isInput_ ? "输入" : "输出") << "设备: " << deviceInfo->name << std::endl;
    }
    
    // 输出所有设备
    for (auto i = 0; i < numDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo) {
            std::cout << "设备 #" << i << ": " << deviceInfo->name << std::endl;
        }
    }
}

PortAudioDevice::~PortAudioDevice() {
    stop();
    
    // 减少实例计数
    PortAudioManager::getInstance().decrementCount();
    
    // 只有当没有更多实例时才终止PortAudio
    if (PortAudioManager::getInstance().getCount() <= 0) {
        PortAudioManager::getInstance().~PortAudioManager();
    }
}

bool PortAudioDevice::initialize(int sampleRate, int channels) {
    sampleRate_ = sampleRate;
    channels_ = channels;
    
    try {
        PaDeviceIndex deviceIndex = isInput_ ? Pa_GetDefaultInputDevice() : Pa_GetDefaultOutputDevice();
        if (deviceIndex == paNoDevice) {
            std::cerr << "没有找到默认" << (isInput_ ? "输入" : "输出") << "设备" << std::endl;
            return false;
        }
        
        PaStreamParameters parameters{};
        parameters.device = deviceIndex;
        parameters.channelCount = channels_;
        parameters.sampleFormat = paFloat32;
        
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(deviceIndex);
        if (!deviceInfo) {
            std::cerr << "无法获取设备信息" << std::endl;
            return false;
        }
        
        parameters.suggestedLatency = isInput_ ?
            deviceInfo->defaultLowInputLatency :
            deviceInfo->defaultLowOutputLatency;
        parameters.hostApiSpecificStreamInfo = nullptr;

        PaError err = Pa_OpenStream(
            &stream_,
            isInput_ ? &parameters : nullptr,    // 输入参数
            isInput_ ? nullptr : &parameters,    // 输出参数
            sampleRate_,                         // 采样率
            BUFFER_SIZE,                         // 每次回调的帧数
            paClipOff,                          // 不裁剪
            audioCallback,                       // 回调函数
            this                                // 用户数据
        );

        if (err != paNoError) {
            std::cerr << "打开音频流失败: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }
        
        initialized_ = true;
        std::cout << "音频设备初始化成功: " << sampleRate_ << "Hz, " << channels_ << " 通道" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "初始化音频设备时发生异常: " << e.what() << std::endl;
        return false;
    }
}

bool PortAudioDevice::start() {
    if (!stream_) {
        std::cerr << "音频流未初始化" << std::endl;
        return false;
    }
    
    PaError err = Pa_StartStream(stream_);
    if (err != paNoError) {
        std::cerr << "启动音频流失败: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    std::cout << "音频流启动成功" << std::endl;
    return true;
}

bool PortAudioDevice::stop() {
    if (!stream_) return false;
    
    PaError err = Pa_StopStream(stream_);
    if (err != paNoError) {
        std::cerr << "停止音频流失败: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    err = Pa_CloseStream(stream_);
    if (err != paNoError) {
        std::cerr << "关闭音频流失败: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    stream_ = nullptr;
    initialized_ = false;
    std::cout << "音频流已停止并关闭" << std::endl;
    return true;
}

void PortAudioDevice::setCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = std::move(callback);
}

void PortAudioDevice::processAudio(const float* input, float* output, unsigned long frameCount) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isInput_) {
        // 输入设备：将输入数据应用音量后传递给回调
        if (input && callback_) {
            std::vector<float> audioData(frameCount * channels_);
            
            // 应用音量
            for (unsigned long i = 0; i < frameCount * channels_; i++) {
                audioData[i] = input[i] * volume_;
            }
            
            callback_(audioData);
        }
    } else {
        // 输出设备：将缓冲区数据应用音量后输出
        if (output) {
            // 清空输出缓冲区
            std::fill(output, output + frameCount * channels_, 0.0f);
            
            // 如果有数据在缓冲区，应用音量并输出
            size_t copySize = std::min(static_cast<size_t>(frameCount * channels_), buffer_.size());
            if (copySize > 0) {
                for (size_t i = 0; i < copySize; i++) {
                    output[i] = buffer_[i] * volume_;
                }
                
                // 移除已处理的数据
                buffer_.erase(buffer_.begin(), buffer_.begin() + copySize);
            }
        }
    }
}

bool PortAudioDevice::play(const std::vector<float>& data) {
    if (isInput_ || !initialized_) {
        return false;
    }
    
    try {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 限制缓冲区大小，防止内存溢出
        const size_t MAX_BUFFER_SIZE = 48000 * 2; // 限制为约1秒的音频数据
        
        // 如果缓冲区已经很大，直接清空旧数据
        if (buffer_.size() > MAX_BUFFER_SIZE) {
            buffer_.clear();
            std::cout << "警告：缓冲区过大，已清空" << std::endl;
        }
        
        // 只添加有限大小的数据
        size_t dataToAdd = std::min(data.size(), MAX_BUFFER_SIZE);
        buffer_.insert(buffer_.end(), data.begin(), data.begin() + dataToAdd);
        
        // 打印调试信息
        std::cout << "添加音频数据到缓冲区，大小: " << dataToAdd << "，当前缓冲区大小: " << buffer_.size() << std::endl;
        
        // 打印前10个样本值，用于验证
        std::cout << "样本值: ";
        for (size_t i = 0; i < std::min(size_t(10), data.size()); i++) {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "播放音频数据失败: " << e.what() << std::endl;
        return false;
    }
}

// 获取可用的音频设备列表
std::vector<AudioDeviceInfo> PortAudioDevice::getAvailableDevices(bool isInput) {
    std::vector<AudioDeviceInfo> devices;
    
    // 确保PortAudio已初始化
    if (!PortAudioManager::getInstance().initialize()) {
        std::cerr << "PortAudio 初始化失败" << std::endl;
        return devices;
    }
    
    // 获取设备数量
    int numDevices = Pa_GetDeviceCount();
    if (numDevices <= 0) {
        std::cerr << "没有找到音频设备: " << Pa_GetErrorText(numDevices) << std::endl;
        return devices;
    }
    
    std::cout << "PortAudio 发现 " << numDevices << " 个音频设备" << std::endl;
    
    // 获取默认设备
    PaDeviceIndex defaultInputDeviceIndex = Pa_GetDefaultInputDevice();
    PaDeviceIndex defaultOutputDeviceIndex = Pa_GetDefaultOutputDevice();
    
    std::cout << "默认输入设备索引: " << defaultInputDeviceIndex << std::endl;
    std::cout << "默认输出设备索引: " << defaultOutputDeviceIndex << std::endl;
    
    // 如果系统有默认设备，确保它出现在列表的最前面
    if (isInput && defaultInputDeviceIndex != paNoDevice) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(defaultInputDeviceIndex);
        if (deviceInfo && deviceInfo->maxInputChannels > 0) {
            AudioDeviceInfo info;
            info.index = defaultInputDeviceIndex;
            info.name = std::string("默认: ") + deviceInfo->name;
            info.maxInputChannels = deviceInfo->maxInputChannels;
            info.maxOutputChannels = deviceInfo->maxOutputChannels;
            info.defaultSampleRate = deviceInfo->defaultSampleRate;
            devices.push_back(info);
            
            std::cout << "添加默认输入设备: " << info.name << std::endl;
        }
    } else if (!isInput && defaultOutputDeviceIndex != paNoDevice) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(defaultOutputDeviceIndex);
        if (deviceInfo && deviceInfo->maxOutputChannels > 0) {
            AudioDeviceInfo info;
            info.index = defaultOutputDeviceIndex;
            info.name = std::string("默认: ") + deviceInfo->name;
            info.maxInputChannels = deviceInfo->maxInputChannels;
            info.maxOutputChannels = deviceInfo->maxOutputChannels;
            info.defaultSampleRate = deviceInfo->defaultSampleRate;
            devices.push_back(info);
            
            std::cout << "添加默认输出设备: " << info.name << std::endl;
        }
    }
    
    // 遍历所有设备
    for (int i = 0; i < numDevices; i++) {
        // 跳过已经添加的默认设备
        if ((isInput && i == defaultInputDeviceIndex) || (!isInput && i == defaultOutputDeviceIndex)) {
            continue;
        }
        
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo) {
            // 检查设备是否支持所需的输入/输出
            if ((isInput && deviceInfo->maxInputChannels > 0) ||
                (!isInput && deviceInfo->maxOutputChannels > 0)) {
                AudioDeviceInfo info;
                info.index = i;
                info.name = deviceInfo->name;
                info.maxInputChannels = deviceInfo->maxInputChannels;
                info.maxOutputChannels = deviceInfo->maxOutputChannels;
                info.defaultSampleRate = deviceInfo->defaultSampleRate;
                devices.push_back(info);
                
                std::cout << "添加" << (isInput ? "输入" : "输出") << "设备: " << info.name << std::endl;
            }
        }
    }
    
    // 如果没有找到任何设备，添加一个虚拟设备
    if (devices.empty()) {
        std::cout << "未找到任何" << (isInput ? "输入" : "输出") << "设备，添加虚拟设备" << std::endl;
        AudioDeviceInfo virtualDevice;
        virtualDevice.index = isInput ? defaultInputDeviceIndex : defaultOutputDeviceIndex;
        if (virtualDevice.index == paNoDevice) virtualDevice.index = 0;
        virtualDevice.name = isInput ? "默认音频输入" : "默认音频输出";
        virtualDevice.maxInputChannels = isInput ? 2 : 0;
        virtualDevice.maxOutputChannels = isInput ? 0 : 2;
        virtualDevice.defaultSampleRate = 44100;
        devices.push_back(virtualDevice);
    }
    
    return devices;
}

// 设置音频设备
bool PortAudioDevice::setDevice(int deviceIndex) {
    // 如果流已经在运行，需要先停止
    bool wasRunning = stream_ != nullptr;
    if (wasRunning) {
        stop();
    }
    
    // 检查设备索引是否有效
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(deviceIndex);
    if (!deviceInfo) {
        std::cerr << "无效的设备索引: " << deviceIndex << std::endl;
        return false;
    }
    
    // 检查设备是否支持所需的输入/输出
    if ((isInput_ && deviceInfo->maxInputChannels < channels_) ||
        (!isInput_ && deviceInfo->maxOutputChannels < channels_)) {
        std::cerr << "设备不支持所需的通道数: " << channels_ << std::endl;
        return false;
    }
    
    // 更新当前设备索引
    currentDeviceIndex_ = deviceIndex;
    
    // 如果之前在运行，重新初始化并启动
    if (wasRunning) {
        if (!initialize(sampleRate_, channels_)) {
            return false;
        }
        return start();
    }
    
    return true;
}

// 设置音量
void PortAudioDevice::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 1.0f);
}

float PortAudioDevice::getVolume() const {
    return volume_;
}

// 在audioCallback中应用音量
int PortAudioDevice::audioCallback(const void* input, void* output,
                                 unsigned long frameCount,
                                 [[maybe_unused]] const PaStreamCallbackTimeInfo* timeInfo,
                                 [[maybe_unused]] PaStreamCallbackFlags statusFlags,
                                 void* userData) {
    auto* device = static_cast<PortAudioDevice*>(userData);
    
    if (device->isInput_) {
        // 输入设备：应用输入音量
        if (device->callback_ && input) {
            const float* inputBuffer = static_cast<const float*>(input);
            std::vector<float> processedData(frameCount * device->channels_);
            
            // 应用音量
            for (unsigned long i = 0; i < frameCount * device->channels_; ++i) {
                processedData[i] = inputBuffer[i] * device->volume_;
            }
            
            device->callback_(processedData);
        }
    } else {
        // 输出设备：应用输出音量
        if (output) {
            float* outputBuffer = static_cast<float*>(output);
            // 清空输出缓冲区
            std::fill_n(outputBuffer, frameCount * device->channels_, 0.0f);
            
            if (device->callback_) {
                std::vector<float> data(frameCount * device->channels_);
                device->callback_(data);
                
                // 应用音量
                for (unsigned long i = 0; i < frameCount * device->channels_; ++i) {
                    outputBuffer[i] = data[i] * device->volume_;
                }
            }
        }
    }
    
    return paContinue;
}

} // namespace voicechat 