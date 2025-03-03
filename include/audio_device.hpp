#pragma once

#include "audio_interface.hpp"
#include <portaudio.h>
#include <memory>
#include <vector>
#include <mutex>

namespace voicechat {

class PortAudioDevice : public IAudioDevice {
public:
    PortAudioDevice(bool isInput);
    ~PortAudioDevice() override;

    bool initialize(int sampleRate, int channels) override;
    bool start() override;
    bool stop() override;
    void setCallback(AudioCallback callback) override;

private:
    // PortAudio回调函数
    static int paCallback(const void* inputBuffer, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void* userData);

    // 处理音频数据
    void processAudio(const float* input, float* output, unsigned long frameCount);

private:
    bool isInput_;                    // 是否为输入设备
    int sampleRate_;                  // 采样率
    int channels_;                    // 通道数
    PaStream* stream_;               // PortAudio流
    AudioCallback callback_;          // 音频回调函数
    std::mutex mutex_;               // 互斥锁
    std::vector<float> buffer_;      // 音频缓冲区
    static constexpr size_t BUFFER_SIZE = 1024;  // 缓冲区大小
};

} // namespace voicechat 