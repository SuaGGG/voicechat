#pragma once

#include "audio_interface.hpp"
#include <portaudio.h>
#include <memory>
#include <vector>
#include <mutex>
#include <iostream>
#include <string>
#include <unordered_map>

namespace voicechat {

// 音频设备信息结构体
struct AudioDeviceInfo {
    int index;
    std::string name;
    int maxInputChannels;
    int maxOutputChannels;
    double defaultSampleRate;
};

class PortAudioDevice : public IAudioDevice {
public:
    PortAudioDevice(bool isInput);
    ~PortAudioDevice() override;

    bool initialize(int sampleRate, int channels) override;
    bool start() override;
    bool stop() override;
    void setCallback(AudioCallback callback) override;
    
    // 检查设备是否成功初始化
    bool isInitialized() const { return initialized_; }
    
    // 直接播放音频数据（仅适用于输出设备）
    bool play(const std::vector<float>& data);
    
    // 获取可用的音频设备列表
    static std::vector<AudioDeviceInfo> getAvailableDevices(bool isInput);
    
    // 设置音频设备
    bool setDevice(int deviceIndex);
    
    // 获取当前设备索引
    int getCurrentDeviceIndex() const { return currentDeviceIndex_; }
    
    // 音量控制方法实现
    void setVolume(float volume) override;
    float getVolume() const override;

private:
    static constexpr size_t BUFFER_SIZE = 1024;  // 缓冲区大小

    bool initialized_ = false;
    bool isInput_;
    int currentDeviceIndex_ = -1;
    int sampleRate_ = 44100;
    int channels_ = 2;
    float volume_ = 1.0f;
    AudioCallback callback_;
    PaStream* stream_ = nullptr;
    std::mutex mutex_;
    std::vector<float> buffer_;
    
    // 音频处理方法
    void processAudio(const float* input, float* output, unsigned long frameCount);
    
    // PortAudio回调函数
    static int audioCallback(const void* input, void* output,
                           unsigned long frameCount,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData);
};

// 虚拟音频设备类，用于在没有实际音频设备时提供模拟功能
class NullAudioDevice : public IAudioDevice {
public:
    NullAudioDevice(bool isInput) : isInput_(isInput) {
        std::cout << "创建虚拟" << (isInput_ ? "输入" : "输出") << "音频设备" << std::endl;
    }
    
    ~NullAudioDevice() override = default;
    
    bool initialize(int sampleRate, int channels) override {
        sampleRate_ = sampleRate;
        channels_ = channels;
        std::cout << "初始化虚拟音频设备: " << sampleRate << "Hz, " << channels << " 通道" << std::endl;
        return true;
    }
    
    bool start() override {
        std::cout << "启动虚拟音频设备" << std::endl;
        return true;
    }
    
    bool stop() override {
        std::cout << "停止虚拟音频设备" << std::endl;
        return true;
    }
    
    void setCallback(AudioCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        callback_ = std::move(callback);
    }
    
    // 虚拟设备的音量控制
    void setVolume(float volume) {
        volume_ = volume;
        std::cout << "设置虚拟音频设备音量: " << volume << std::endl;
    }
    
    float getVolume() const { return volume_; }
    
private:
    bool isInput_;
    int sampleRate_ = 48000;
    int channels_ = 1;
    AudioCallback callback_;
    std::mutex mutex_;
    float volume_ = 1.0f;
};

} // namespace voicechat 