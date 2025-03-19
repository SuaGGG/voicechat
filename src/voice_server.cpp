#include "voice_server.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace voicechat {

const std::string MAIN_CHANNEL = "main";  // 定义主频道ID

// // 添加辅助函数，用于发送带消息头的数据
// void sendWithHeader(AsioServer* server, const std::string& clientId, const std::vector<uint8_t>& data) {
//     // 准备完整的消息（4字节头 + 数据）
//     std::vector<uint8_t> packet;
//     packet.reserve(4 + data.size());
    
//     // 添加长度头（4字节，小端序）
//     uint32_t size = static_cast<uint32_t>(data.size());
//     for (int i = 0; i < 4; ++i) {
//         packet.push_back(static_cast<uint8_t>((size >> (i * 8)) & 0xFF));
//     }
    
//     // 添加数据
//     packet.insert(packet.end(), data.begin(), data.end());
    
//     // 发送数据
//     server->sendTo(clientId, packet);
// }

VoiceServer::VoiceServer(uint16_t port)
    : port_(port), running_(false), server_(std::make_unique<AsioServer>()) {
    // 创建主频道
    rooms_[MAIN_CHANNEL] = std::unordered_set<std::string>();
    std::cout << "创建主频道: " << MAIN_CHANNEL << std::endl;
}

VoiceServer::~VoiceServer() {
    stop();
}

bool VoiceServer::initialize(uint16_t port) {
    port_ = port;
    return true;
}

bool VoiceServer::start() {
    if (running_) {
        return false;
    }
    
    try {
        // 设置消息回调
        server_->setMessageCallback([this](const std::string& clientId, const std::vector<uint8_t>& data) {
            onMessage(clientId, data);
        });

        // 设置客户端连接回调
        server_->setClientConnectedCallback([this](const std::string& clientId) {
            onClientConnected(clientId);
        });

        // 设置客户端断开连接回调
        server_->setClientDisconnectedCallback([this](const std::string& clientId) {
            onClientDisconnected(clientId);
        });

        server_->start(port_);
        running_ = true;
        std::cout << "服务器启动成功，监听端口: " << port_ << std::endl;
        std::cout << "主频道已开启，等待客户端连接..." << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        return false;
    }
}

void VoiceServer::stop() {
    if (!running_) {
        return;
    }
    
    try {
        server_->stop();
    } catch (const std::exception& e) {
        std::cerr << "Error while stopping server: " << e.what() << std::endl;
    }
    running_ = false;
}

size_t VoiceServer::getConnectedClientsCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return clientRooms_.size();
}

size_t VoiceServer::getRoomParticipantsCount(const std::string& roomId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto it = rooms_.find(roomId); it != rooms_.end()) {
        return it->second.size();
    }
    return 0;
}

std::unordered_map<std::string, size_t> VoiceServer::getRoomParticipantCounts() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::unordered_map<std::string, size_t> counts;
    for (const auto& [roomId, participants] : rooms_) {
        counts[roomId] = participants.size();
    }
    return counts;
}

void VoiceServer::onClientConnected(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 将客户端加入主频道
    clientRooms_[clientId] = MAIN_CHANNEL;
    rooms_[MAIN_CHANNEL].insert(clientId);
    
    std::cout << "客户端已连接: " << clientId << " (自动加入主频道)" << std::endl;

    // 发送欢迎消息
    try {
        ServerResponse response;
        response.set_status(ServerResponse::SUCCESS);
        response.set_message("欢迎来到语音聊天服务器！已自动加入主频道");
        
        std::vector<uint8_t> data(response.ByteSizeLong());
        response.SerializeToArray(data.data(), static_cast<int>(data.size()));
        server_->sendTo(clientId, data);
    } catch (const std::exception& e) {
        std::cerr << "发送欢迎消息失败: " << e.what() << std::endl;
    }
}

void VoiceServer::onClientDisconnected(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto it = clientRooms_.find(clientId); it != clientRooms_.end()) {
        auto roomId = it->second;
        rooms_[roomId].erase(clientId);
        if (rooms_[roomId].empty() && roomId != MAIN_CHANNEL) {  // 不删除主频道
            rooms_.erase(roomId);
        }
        clientRooms_.erase(it);
    }
    std::cout << "客户端断开连接: " << clientId << std::endl;
}

void VoiceServer::onMessage(const std::string& clientId, const std::vector<uint8_t>& data) {
    try {
        std::cout << "\n=== 开始处理客户端消息 ===" << std::endl;
        std::cout << "客户端ID: " << clientId << std::endl;
        std::cout << "消息大小: " << data.size() << " 字节" << std::endl;
        
        // 打印原始数据
        std::cout << "原始消息内容（十六进制）: ";
        for (const auto& byte : data) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
        std::cout << std::dec << std::endl;
        
        // 尝试解析为控制消息
        voicechat::ControlMessage controlMsg;
        if (controlMsg.ParseFromArray(data.data(), static_cast<int>(data.size()))) {
            std::cout << "成功解析为控制消息:" << std::endl;
            std::cout << "- 类型: " << controlMsg.type() << std::endl;
            std::cout << "- 用户ID: " << controlMsg.user_id() << std::endl;
            std::cout << "- 请求ID: " << controlMsg.request_id() << std::endl;
            if (!controlMsg.room_id().empty()) {
                std::cout << "- 房间ID: " << controlMsg.room_id() << std::endl;
            }
            handleControlMessage(clientId, controlMsg);
            std::cout << "=== 消息处理完成 ===\n" << std::endl;
            return;
        }
        std::cout << "不是控制消息，尝试解析为音频消息" << std::endl;

        // 尝试解析为音频消息
        voicechat::AudioData audioMsg;
        if (audioMsg.ParseFromArray(data.data(), static_cast<int>(data.size()))) {
            std::cout << "成功解析为音频消息，来自用户: " << audioMsg.user_id() << std::endl;
            handleAudioData(clientId, audioMsg);
            std::cout << "=== 消息处理完成 ===\n" << std::endl;
            return;
        }

        std::cerr << "无法解析消息类型" << std::endl;
        std::cout << "=== 消息处理失败 ===\n" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "处理客户端 " << clientId << " 的消息时发生错误: " << e.what() << std::endl;
        std::cout << "=== 消息处理出错 ===\n" << std::endl;
    }
}


void VoiceServer::handleControlMessage(const std::string& clientId, const voicechat::ControlMessage& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    switch (msg.type()) {
        case ControlMessage::LIST_ROOMS: {
            std::cout << "收到获取房间列表请求，来自客户端: " << clientId << std::endl;
            std::cout << "- 请求ID: " << msg.request_id() << std::endl;
            
            // 构建房间列表响应
            std::ostringstream oss;
            for (const auto& [roomId, participants] : rooms_) { // 小于1000房间可用，过大效率低下
                if (!oss.str().empty()) {
                    oss << ";"; // 使用分号分隔多个房间
                }
                oss << roomId << ":";
                // 传入房间用户名
                for (const auto& clientId : participants) {
                    oss << clientId << ":";
                }
                oss << ".";
                std::cout << "添加房间到列表: " << roomId << " (在线人数: " << participants.size() << ")" << std::endl;
            }
            
            // 发送响应
            try {
                ServerResponse response;
                response.set_status(ServerResponse::SUCCESS);
                response.set_message(oss.str());
                response.set_request_id(msg.request_id());
                
                std::vector<uint8_t> data(response.ByteSizeLong());
                if (!response.SerializeToArray(data.data(), static_cast<int>(data.size()))) {
                    throw std::runtime_error("序列化响应消息失败");
                }
                
                std::cout << "准备发送房间列表响应:" << std::endl;
                std::cout << "- 请求ID: " << msg.request_id() << std::endl;
                std::cout << "- 消息内容: " << oss.str() << std::endl;
                std::cout << "- 序列化后大小: " << data.size() << " 字节" << std::endl;
                
                server_->sendTo(clientId, data);
                std::cout << "已发送房间列表响应" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "发送房间列表失败: " << e.what() << std::endl;
            }
            break;
        }
        case ControlMessage::JOIN: {
            std::string roomId = msg.room_id();
            if (roomId.empty()) {
                roomId = MAIN_CHANNEL;  // 如果没有指定房间，使用主频道
            }
            
            // 如果客户端已在某个房间，先离开该房间
            if (auto it = clientRooms_.find(clientId); it != clientRooms_.end()) {
                rooms_[it->second].erase(clientId);
                if (rooms_[it->second].empty() && it->second != MAIN_CHANNEL) {  // 不删除主频道
                    rooms_.erase(it->second);
                }
            }
            
            // 加入新房间
            clientRooms_[clientId] = roomId;
            rooms_[roomId].insert(clientId);
            std::cout << "客户端 " << clientId << " 加入房间: " << roomId << std::endl;
            
            // 发送确认消息
            try {
                ServerResponse response;
                response.set_status(ServerResponse::SUCCESS);
                response.set_message("成功加入房间: " + roomId);
                response.set_request_id(msg.request_id());  // 设置请求ID
                
                std::vector<uint8_t> data(response.ByteSizeLong());
                response.SerializeToArray(data.data(), static_cast<int>(data.size()));
                server_->sendTo(clientId, data);
            } catch (const std::exception& e) {
                std::cerr << "发送房间加入确认消息失败: " << e.what() << std::endl;
            }
            break;
        }
        case ControlMessage::LEAVE: {
            if (auto it = clientRooms_.find(clientId); it != clientRooms_.end()) {
                std::string oldRoom = it->second;
                rooms_[oldRoom].erase(clientId);
                if (rooms_[oldRoom].empty() && oldRoom != MAIN_CHANNEL) {  // 不删除主频道
                    rooms_.erase(oldRoom);
                }
                
                // 离开当前房间后自动回到主频道
                clientRooms_[clientId] = MAIN_CHANNEL;
                rooms_[MAIN_CHANNEL].insert(clientId);
                
                std::cout << "客户端 " << clientId << " 离开房间: " << oldRoom << " (自动回到主频道)" << std::endl;
                
                // 发送确认消息
                try {
                    ServerResponse response;
                    response.set_status(ServerResponse::SUCCESS);
                    response.set_message("已离开房间: " + oldRoom + "，回到主频道");
                    response.set_request_id(msg.request_id());  // 设置请求ID
                    
                    std::vector<uint8_t> data(response.ByteSizeLong());
                    response.SerializeToArray(data.data(), static_cast<int>(data.size()));
                    server_->sendTo(clientId, data);
                } catch (const std::exception& e) {
                    std::cerr << "发送房间离开确认消息失败: " << e.what() << std::endl;
                }
            }
            break;
        }
        default:
            std::cerr << "未知的控制消息类型，来自客户端: " << clientId << std::endl;
            break;
    }
}


void VoiceServer::handleAudioData(const std::string& clientId, const voicechat::AudioData& audioData) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto it = clientRooms_.find(clientId); it != clientRooms_.end()) {
        std::vector<uint8_t> serializedData(audioData.ByteSizeLong());
        if (!audioData.SerializeToArray(serializedData.data(), static_cast<int>(serializedData.size()))) {
            std::cerr << "Failed to serialize audio data from client: " << clientId << std::endl;
            return;
        }
        broadcastToRoom(it->second, serializedData, clientId);
    }
}

void VoiceServer::broadcastToRoom(const std::string& roomId, const std::vector<uint8_t>& data, const std::string& excludeClientId) {
    if (auto it = rooms_.find(roomId); it != rooms_.end()) {
        for (const auto& clientId : it->second) {
            if (clientId != excludeClientId) {
                try {
                    server_->sendTo(clientId, data);
                } catch (const std::exception& e) {
                    std::cerr << "Failed to send data to client " << clientId << ": " << e.what() << std::endl;
                }
            }
        }
    }
}

} // namespace voicechat 