#pragma once

#include "audio_interface.hpp"
#include "network_interface.hpp"
#include "asio_network.hpp"
#include "audio_device.hpp"
#include "opus_codec.hpp"
#include "voice_message.pb.h"

#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <future>
#include <vector>

namespace voicechat {

class VoiceClient {
public:
    explicit VoiceClient(const std::string& userId);
    ~VoiceClient();
    
    bool connect(const std::string& host, uint16_t port);
    void disconnect();
    
    bool joinRoom(const std::string& roomId);
    bool leaveRoom();
    
    void setMuted(bool muted);
    bool isMuted() const;
    
    std::unordered_map<std::string, std::vector<std::string>> getAvailableRooms();
    
    std::optional<std::string> getCurrentRoomId() const;

    // 音频设备控制方法
    std::vector<AudioDeviceInfo> getAudioInputDevices() const;
    std::vector<AudioDeviceInfo> getAudioOutputDevices() const;
    bool setAudioInputDevice(int deviceIndex);
    bool setAudioOutputDevice(int deviceIndex);
    void setInputVolume(float volume);
    void setOutputVolume(float volume);
    float getInputVolume() const;
    float getOutputVolume() const;
    
    // 获取音频设备实例
    PortAudioDevice* getInputDevice() const;
    PortAudioDevice* getOutputDevice() const;
    
private:
    void onMessage(const std::vector<uint8_t>& data);
    bool initAudio();
    bool sendRequest(const ControlMessage& request, ServerResponse& response);
    void handleAudioData(const AudioData& audioData);
    void handleServerResponse(const ServerResponse& response);
    void onAudioData(const std::vector<uint8_t>& audioData);
    
    std::string userId_;
    std::unique_ptr<AsioConnection> connection_;
    std::unique_ptr<IAudioDevice> audioDevice_;
    std::unique_ptr<IAudioCodec> audioCodec_;
    std::optional<std::string> currentRoomId_;
    bool muted_;
    bool running_;
    
    // 虚拟音频模式标志
    bool useVirtualAudio_ = false;
    
    // 用于请求-响应模式
    std::atomic<uint32_t> requestIdCounter_{1};
    std::mutex responseMutex_;
    std::unordered_map<uint32_t, std::shared_ptr<std::promise<ServerResponse>>> pendingPromises_;
};

} // namespace voicechat 