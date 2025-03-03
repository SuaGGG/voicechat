#include "audio_device.hpp"
#include <stdexcept>
#include <iostream>

namespace voicechat {

PortAudioDevice::PortAudioDevice(bool isInput)
    : isInput_(isInput)
    , sampleRate_(44100)
    , channels_(2)
    , stream_(nullptr)
    , buffer_(BUFFER_SIZE)
{
    // 初始化PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error("Failed to initialize PortAudio: " + std::string(Pa_GetErrorText(err)));
    }
}

PortAudioDevice::~PortAudioDevice() {
    stop();
    Pa_Terminate();
}

bool PortAudioDevice::initialize(int sampleRate, int channels) {
    sampleRate_ = sampleRate;
    channels_ = channels;
    
    PaStreamParameters parameters{};
    parameters.device = isInput_ ? Pa_GetDefaultInputDevice() : Pa_GetDefaultOutputDevice();
    parameters.channelCount = channels_;
    parameters.sampleFormat = paFloat32;
    parameters.suggestedLatency = isInput_ ?
        Pa_GetDeviceInfo(parameters.device)->defaultLowInputLatency :
        Pa_GetDeviceInfo(parameters.device)->defaultLowOutputLatency;
    parameters.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(
        &stream_,
        isInput_ ? &parameters : nullptr,    // 输入参数
        isInput_ ? nullptr : &parameters,    // 输出参数
        sampleRate_,                         // 采样率
        BUFFER_SIZE,                         // 每次回调的帧数
        paClipOff,                          // 不裁剪
        paCallback,                         // 回调函数
        this                                // 用户数据
    );

    return err == paNoError;
}

bool PortAudioDevice::start() {
    if (!stream_) return false;
    
    PaError err = Pa_StartStream(stream_);
    return err == paNoError;
}

bool PortAudioDevice::stop() {
    if (!stream_) return false;
    
    PaError err = Pa_StopStream(stream_);
    if (err == paNoError) {
        err = Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
    return err == paNoError;
}

void PortAudioDevice::setCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = std::move(callback);
}

int PortAudioDevice::paCallback(const void* inputBuffer, void* outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void* userData) {
    auto* device = static_cast<PortAudioDevice*>(userData);
    device->processAudio(
        static_cast<const float*>(inputBuffer),
        static_cast<float*>(outputBuffer),
        framesPerBuffer
    );
    return paContinue;
}

void PortAudioDevice::processAudio(const float* input, float* output, unsigned long frameCount) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isInput_ && callback_ && input) {
        // 处理输入
        std::vector<float> inputData(input, input + frameCount * channels_);
        callback_(inputData);
    } else if (!isInput_ && output) {
        // 处理输出
        std::fill_n(output, frameCount * channels_, 0.0f);
        if (!buffer_.empty()) {
            size_t copySize = std::min(buffer_.size(), static_cast<size_t>(frameCount * channels_));
            std::copy_n(buffer_.begin(), copySize, output);
            buffer_.erase(buffer_.begin(), buffer_.begin() + copySize);
        }
    }
}

} // namespace voicechat 