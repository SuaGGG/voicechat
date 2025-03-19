#pragma once

#include <vector>
#include <functional>
#include <memory>

namespace voicechat {

// 音频回调函数类型定义
using AudioCallback = std::function<void(const std::vector<float>&)>;

// 音频设备接口
class IAudioDevice {
public:
    virtual ~IAudioDevice() = default;
    
    // 初始化音频设备
    virtual bool initialize(int sampleRate, int channels) = 0;
    
    // 启动音频流
    virtual bool start() = 0;
    
    // 停止音频流
    virtual bool stop() = 0;
    
    // 设置音频回调
    virtual void setCallback(AudioCallback callback) = 0;

    // 音量控制方法
    virtual void setVolume(float volume) = 0;
    virtual float getVolume() const = 0;
};

// 音频编解码器接口
class IAudioCodec {
public:
    virtual ~IAudioCodec() = default;
    
    // 初始化编解码器
    virtual bool initialize(int sampleRate, int channels) = 0;
    
    // 编码音频数据
    virtual std::vector<uint8_t> encode(const std::vector<float>& pcmData) = 0;
    
    // 解码音频数据
    virtual std::vector<float> decode(const std::vector<uint8_t>& encodedData) = 0;
};

// 音频管理器接口
class IAudioManager {
public:
    virtual ~IAudioManager() = default;
    
    // 初始化音频系统
    virtual bool initialize() = 0;
    
    // 开始录音
    virtual bool startRecording() = 0;
    
    // 停止录音
    virtual bool stopRecording() = 0;
    
    // 开始播放
    virtual bool startPlayback() = 0;
    
    // 停止播放
    virtual bool stopPlayback() = 0;
    
    // 设置音频输入回调
    virtual void setInputCallback(AudioCallback callback) = 0;
    
    // 播放音频数据
    virtual void playAudio(const std::vector<float>& audioData) = 0;
};

} // namespace voicechat 