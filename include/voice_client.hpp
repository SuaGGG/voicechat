#pragma once

#include "network_interface.hpp"
#include "voice_message.pb.h"
#include "audio_interface.hpp"
#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>
#include "asio_network.hpp"
#include "audio_device.hpp"
#include "opus_codec.hpp"
#include <unordered_map>

namespace voicechat {

class VoiceClient {
public:
    explicit VoiceClient(const std::string& userId);
    ~VoiceClient();

    // 连接到服务器
    bool connect(const std::string& host, uint16_t port);
    
    // 断开连接
    void disconnect();
    
    // 加入房间
    bool joinRoom(const std::string& roomId);
    
    // 离开房间
    bool leaveRoom();
    
    // 静音/取消静音
    void setMuted(bool muted);
    bool isMuted() const;

    // 获取用户ID
    const std::string& getUserId() const { return userId_; }
    
    // 获取当前房间ID
    const std::string& getCurrentRoomId() const { return currentRoomId_; }

    // 获取可用频道列表
    std::unordered_map<std::string, size_t> getAvailableRooms();

private:
    // 处理来自服务器的消息
    void onMessage(const std::vector<uint8_t>& data);
    
    // 处理音频数据
    void handleAudioData(const AudioData& audioData);
    
    // 处理服务器响应
    void handleServerResponse(const ServerResponse& response);
    
    // 音频回调
    void onAudioData(const std::vector<uint8_t>& audioData);

    // 请求并等待服务器响应（带返回值的版本）
    bool sendRequest(const ControlMessage& request, ServerResponse& response);
    
    // 请求并等待服务器响应（无返回值的版本）
    void sendRequest(const ControlMessage& request);

    std::string userId_;
    std::string currentRoomId_;
    bool muted_;
    bool running_;
    mutable std::mutex mutex_;
    std::unique_ptr<AsioConnection> connection_;
    std::unique_ptr<PortAudioDevice> audioDevice_;
    std::unique_ptr<OpusCodec> audioCodec_;
    
    // 存储服务器响应的Promise
    std::shared_ptr<std::promise<ServerResponse>> responsePromise_;
    std::mutex responseMutex_;
};

} // namespace voicechat 