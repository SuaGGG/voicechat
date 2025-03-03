#pragma once

#include "network_interface.hpp"
#include "voice_message.pb.h"
#include "audio_interface.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <vector>
#include <cstdint>
#include <iostream>
#include <unordered_set>
#include <thread>
#include <chrono>
#include "asio_network.hpp"

namespace voicechat {

struct ClientInfo {
    std::string userId;
    std::string roomId;
    bool muted;
};

class VoiceServer {
public:
    explicit VoiceServer(uint16_t port);
    explicit VoiceServer();
    ~VoiceServer();

    // 初始化服务器
    bool initialize(uint16_t port);
    
    // 启动服务器
    bool start();
    
    // 停止服务器
    void stop();
    
    // 获取当前连接数
    size_t getConnectedClientsCount() const;
    
    // 获取房间内的用户数
    size_t getRoomParticipantsCount(const std::string& roomId) const;

    std::unordered_map<std::string, size_t> getRoomParticipantCounts() const;

private:
    // 处理客户端连接
    void onClientConnected(const std::string& clientId);
    void onClientDisconnected(const std::string& clientId);
    
    // 处理客户端消息
    void onMessage(const std::string& clientId, const std::vector<uint8_t>& data);
    
    // 处理控制消息
    void handleControlMessage(const std::string& clientId, const voicechat::ControlMessage& msg);
    
    // 处理音频数据
    void handleAudioData(const std::string& clientId, const voicechat::AudioData& audioData);
    
    // 广播音频数据到房间
    void broadcastToRoom(const std::string& roomId, const std::vector<uint8_t>& data, const std::string& excludeClientId = "");

    uint16_t port_;
    bool running_;
    mutable std::mutex mutex_;
    std::unique_ptr<AsioServer> server_;
    std::unordered_map<std::string, std::string> clientRooms_;  // clientId -> roomId
    std::unordered_map<std::string, std::unordered_set<std::string>> rooms_;  // roomId -> set of clientIds
};

} // namespace voicechat 