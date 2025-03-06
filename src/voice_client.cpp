#include "voice_client.hpp"
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <thread>
namespace voicechat {

VoiceClient::VoiceClient(const std::string& userId)
    : userId_(userId)
    , muted_(false)
    , running_(false)
{
    try {
        std::cout << "初始化音频设备..." << std::endl;
        
        // 初始化音频设备（作为输入设备）
        audioDevice_ = std::make_unique<PortAudioDevice>(true);
        
        // 初始化音频编解码器
        audioCodec_ = std::make_unique<OpusCodec>();
        if (!audioCodec_->initialize(48000, 1)) { // 48kHz, 单声道
            throw std::runtime_error("初始化音频编解码器失败");
        }
        
        // 设置音频回调
        audioDevice_->setCallback([this](const std::vector<float>& data) {
            if (data.empty()) {
                return;
            }
            // 将float数据转换为uint8_t
            std::vector<uint8_t> audioData(data.size() * sizeof(float));
            std::memcpy(audioData.data(), data.data(), audioData.size());
            onAudioData(audioData);
        });
        
        // 初始化音频设备
        if (!audioDevice_->initialize(48000, 1)) {
            throw std::runtime_error("初始化音频设备失败");
        }
        
        std::cout << "音频设备初始化成功" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "初始化音频设备时发生错误: " << e.what() << std::endl;
        throw; // 重新抛出异常，因为没有音频设备就无法正常工作
    }
}

VoiceClient::~VoiceClient() {
    disconnect();
}

bool VoiceClient::connect(const std::string& host, uint16_t port) {
    try {
        connection_ = std::make_unique<AsioConnection>();
        connection_->setMessageCallback([this](const std::vector<uint8_t>& data) {
            onMessage(data);
        });
        
        if (!connection_->connect(host, port)) {
            std::cerr << "Failed to connect to server" << std::endl;
            return false;
        }

        // 等待一小段时间确保连接建立
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 发送加入请求
        ControlMessage msg;
        msg.set_type(ControlMessage::JOIN);
        msg.set_user_id(userId_);
        
        // 等待服务器响应
        ServerResponse response;
        if (!sendRequest(msg, response)) {
            std::cerr << "等待服务器响应超时" << std::endl;
            return false;
        }

        if (response.status() != ServerResponse::SUCCESS) {
            std::cerr << "加入服务器失败: " << response.message() << std::endl;
            return false;
        }

        running_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in connect: " << e.what() << std::endl;
        return false;
    }
}

void VoiceClient::disconnect() {
    if (connection_) {
        try {
            ControlMessage msg;
            msg.set_type(ControlMessage::LEAVE);
            msg.set_user_id(userId_);
            
            std::vector<uint8_t> data(msg.ByteSizeLong());
            msg.SerializeToArray(data.data(), data.size());
            connection_->send(data);
        } catch (const std::exception& e) {
            std::cerr << "Exception in disconnect: " << e.what() << std::endl;
        }

        connection_.reset();
    }
    
    running_ = false;
    if (!currentRoomId_.empty()) {
        leaveRoom();
    }
}

bool VoiceClient::joinRoom(const std::string& roomId) {
    if (!connection_) {
        std::cerr << "未连接到服务器" << std::endl;
        return false;
    }

    try {
        std::cout << "尝试加入房间: " << roomId << std::endl;
        
        // 创建控制消息
        ControlMessage msg;
        msg.set_type(ControlMessage::JOIN);
        msg.set_user_id(userId_);
        msg.set_room_id(roomId);
        
        // 序列化消息
        std::vector<uint8_t> messageData(msg.ByteSizeLong());
        if (!msg.SerializeToArray(messageData.data(), static_cast<int>(messageData.size()))) {
            throw std::runtime_error("序列化消息失败");
        }
        
        // // 准备完整的消息（4字节头 + 数据）
        // std::vector<uint8_t> packet;
        // packet.reserve(4 + messageData.size());
        
        // // 添加长度头（4字节，小端序）
        // uint32_t size = static_cast<uint32_t>(messageData.size());
        // for (int i = 0; i < 4; ++i) {
        //     packet.push_back(static_cast<uint8_t>((size >> (i * 8)) & 0xFF));
        // }
        
        // // 添加数据
        // packet.insert(packet.end(), messageData.begin(), messageData.end());
        
        // 发送请求
        if (!connection_->send(messageData)) {
            std::cerr << "发送加入房间请求失败" << std::endl;
            return false;
        }
        
        std::cout << "已发送加入房间请求，等待响应..." << std::endl;
        
        // 等待响应（通过onMessage和handleServerResponse处理）
        // TODO: 添加超时机制
        
        currentRoomId_ = roomId;
        std::cout << "成功加入房间: " << roomId << std::endl;
        
        // 启动音频设备
        if (!audioDevice_->start()) {
            std::cerr << "启动音频设备失败，但已加入房间" << std::endl;
            // 不要因为音频设备失败就退出房间
            return true;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "加入房间时发生错误: " << e.what() << std::endl;
        return false;
    }
}

bool VoiceClient::leaveRoom() {
    if (!connection_ || currentRoomId_.empty()) {
        return false;
    }

    try {
        std::cout << "尝试离开房间: " << currentRoomId_ << std::endl;
        
        // 创建控制消息
        ControlMessage msg;
        msg.set_type(ControlMessage::LEAVE);
        msg.set_user_id(userId_);
        msg.set_room_id(currentRoomId_);
        
        // 序列化消息
        std::vector<uint8_t> messageData(msg.ByteSizeLong());
        if (!msg.SerializeToArray(messageData.data(), static_cast<int>(messageData.size()))) {
            throw std::runtime_error("序列化消息失败");
        }
        
        // 准备完整的消息（4字节头 + 数据）
        // std::vector<uint8_t> packet;
        // packet.reserve(4 + messageData.size());
        
        // // 添加长度头（4字节，小端序）
        // uint32_t size = static_cast<uint32_t>(messageData.size());
        // for (int i = 0; i < 4; ++i) {
        //     packet.push_back(static_cast<uint8_t>((size >> (i * 8)) & 0xFF));
        // }
        
        // // 添加数据
        // packet.insert(packet.end(), messageData.begin(), messageData.end());
        
        // 发送请求
        if (!connection_->send(messageData)) {
            std::cerr << "发送离开房间请求失败" << std::endl;
            return false;
        }
        
        std::cout << "已发送离开房间请求" << std::endl;
        currentRoomId_.clear();
        
        // 停止音频设备
        audioDevice_->stop();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "离开房间时发生错误: " << e.what() << std::endl;
        return false;
    }
}

void VoiceClient::setMuted(bool muted) {
    std::lock_guard<std::mutex> lock(mutex_);
    muted_ = muted;
}

bool VoiceClient::isMuted() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return muted_;
}

void VoiceClient::onMessage(const std::vector<uint8_t>& data) {
    try {
        std::cout << "\n=== 开始处理服务器消息 ===" << std::endl;
        std::cout << "收到数据大小: " << data.size() << " 字节" << std::endl;
        
        // 打印原始数据的十六进制表示
        std::cout << "原始消息内容（十六进制）: ";
        for (const auto& byte : data) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
        std::cout << std::dec << std::endl;
        
        // 尝试解析为服务器响应
        ServerResponse response;
        bool parseSuccess = response.ParseFromArray(data.data(), static_cast<int>(data.size()));
        std::cout << "服务器响应解析" << (parseSuccess ? "成功" : "失败") << std::endl;
        
        if (parseSuccess) {
            std::cout << "响应详情:" << std::endl;
            std::cout << "- 状态: " << response.status() << std::endl;
            std::cout << "- 消息: " << response.message() << std::endl;
            std::cout << "- 请求ID: " << response.request_id() << std::endl;
            
            // 检查是否有等待的Promise
            uint32_t requestId = response.request_id();
            std::shared_ptr<std::promise<ServerResponse>> promise;
            {
                std::lock_guard<std::mutex> lock(responseMutex_);
                auto it = pendingPromises_.find(requestId);
                if (it != pendingPromises_.end()) {
                    promise = it->second;
                    pendingPromises_.erase(it);
                    std::cout << "找到对应的Promise，请求ID: " << requestId << std::endl;
                    std::cout << "当前等待中的Promise数量: " << pendingPromises_.size() << std::endl;
                } else {
                    std::cout << "未找到对应的Promise，请求ID: " << requestId << std::endl;
                    std::cout << "当前等待中的Promise数量: " << pendingPromises_.size() << std::endl;
                    for (const auto& [id, _] : pendingPromises_) {
                        std::cout << "- 等待中的请求ID: " << id << std::endl;
                    }
                }
            }
            
            if (promise) {
                try {
                    promise->set_value(response);
                    std::cout << "响应设置完成，请求ID: " << requestId << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "设置Promise值时发生错误: " << e.what() << std::endl;
                }
            } else {
                std::cout << "没有找到匹配的请求ID，这可能是服务器主动发送的消息" << std::endl;
            }
            return;
        }
        
        // 尝试解析为音频数据
        AudioData audioData;
        if (audioData.ParseFromArray(data.data(), static_cast<int>(data.size()))) {
            std::cout << "成功解析为音频消息" << std::endl;
            handleAudioData(audioData);
            return;
        }
        
        std::cerr << "无法解析消息类型" << std::endl;
        std::cout << "=== 消息处理结束 ===\n" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "处理消息时发生异常: " << e.what() << std::endl;
    }
}

void VoiceClient::handleAudioData(const AudioData& audioData) {
    if (audioData.user_id() == userId_) {
        return; // 忽略自己的音频
    }

    try {
        // 获取音频数据
        const std::string& payload = audioData.audio_payload();
        std::vector<uint8_t> encodedData(payload.begin(), payload.end());
        
        // 解码音频数据
        std::vector<float> decodedData = audioCodec_->decode(encodedData);
        
        // 设置回调以播放音频
        audioDevice_->setCallback([](const std::vector<float>& data) {
            // 这里不需要做任何事情，因为我们是在播放模式
        });
        
        // 初始化音频设备（作为输出设备）
        if (!audioDevice_->initialize(48000, 1)) {
            std::cerr << "Failed to initialize audio device for playback" << std::endl;
            return;
        }
        
        // 启动音频设备进行播放
        if (!audioDevice_->start()) {
            std::cerr << "Failed to start audio device for playback" << std::endl;
            return;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in handleAudioData: " << e.what() << std::endl;
    }
}

void VoiceClient::handleServerResponse(const ServerResponse& response) {
    std::lock_guard<std::mutex> lock(responseMutex_);
    if (pendingPromises_[response.request_id()]) {
        pendingPromises_[response.request_id()]->set_value(response);
    }
}

void VoiceClient::onAudioData(const std::vector<uint8_t>& audioData) {
    if (!connection_ || currentRoomId_.empty() || muted_) {
        return;
    }

    try {
        // 将uint8_t数据转换为float
        std::vector<float> floatData(audioData.size() / sizeof(float));
        std::memcpy(floatData.data(), audioData.data(), audioData.size());
        
        // 编码音频数据
        std::vector<uint8_t> encodedData = audioCodec_->encode(floatData);
        
        // 创建音频消息
        AudioData msg;
        msg.set_user_id(userId_);
        msg.set_audio_payload(std::string(encodedData.begin(), encodedData.end()));
        msg.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        msg.set_sequence_number(0); // TODO: 实现序列号
        
        // 序列化音频消息
        std::vector<uint8_t> messageData(msg.ByteSizeLong());
        if (!msg.SerializeToArray(messageData.data(), static_cast<int>(messageData.size()))) {
            std::cerr << "序列化音频消息失败" << std::endl;
            return;
        }
        
        // 发送数据
        connection_->send(messageData);
    } catch (const std::exception& e) {
        std::cerr << "处理音频数据时发生错误: " << e.what() << std::endl;
    }
}

std::unordered_map<std::string, size_t> VoiceClient::getAvailableRooms() {
    std::unordered_map<std::string, size_t> rooms;
    
    try {
        std::cout << "正在获取可用房间列表..." << std::endl;
        
        // 创建请求消息
        ControlMessage request;
        request.set_type(ControlMessage::LIST_ROOMS);
        request.set_user_id(userId_);
        
        // 发送请求并等待响应
        ServerResponse response;
        if (sendRequest(request, response)) {
            if (response.status() == ServerResponse::SUCCESS) {
                std::cout << "收到服务器响应: " << response.message() << std::endl;
                
                // 解析响应消息中的房间信息
                std::string message = response.message();
                std::istringstream iss(message);
                std::string room;
                
                // 按分号分割房间信息
                while (std::getline(iss, room, ';')) {
                    if (room.empty()) continue;
                    
                    size_t pos = room.find(':');
                    if (pos != std::string::npos) {
                        std::string roomId = room.substr(0, pos);
                        std::string countStr = room.substr(pos + 1);
                        
                        // 移除可能的空白字符和换行符
                        roomId.erase(0, roomId.find_first_not_of(" \t\r\n"));
                        roomId.erase(roomId.find_last_not_of(" \t\r\n") + 1);
                        countStr.erase(0, countStr.find_first_not_of(" \t\r\n"));
                        countStr.erase(countStr.find_last_not_of(" \t\r\n") + 1);
                        
                        try {
                            size_t count = std::stoull(countStr);
                            rooms[roomId] = count;
                            std::cout << "解析到房间: " << roomId << " (在线人数: " << count << ")" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "解析房间人数失败: " << countStr << ": " << e.what() << std::endl;
                            continue;
                        }
                    }
                }
            } else {
                std::cout << "获取房间列表失败: " << response.message() << std::endl;
            }
        } else {
            std::cout << "获取房间列表请求超时或失败" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "获取房间列表失败: " << e.what() << std::endl;
    }
    
    if (rooms.empty()) {
        std::cout << "当前没有可用的房间" << std::endl;
    } else {
        std::cout << "\n当前可用房间列表:" << std::endl;
        for (const auto& [roomId, count] : rooms) {
            std::cout << "- " << roomId << " (在线人数: " << count << ")" << std::endl;
        }
        std::cout << std::endl;
    }
    
    return rooms;
}

bool VoiceClient::sendRequest(const ControlMessage& request, ServerResponse& response) {
    if (!connection_) {
        throw std::runtime_error("未连接到服务器");
    }

    // 生成请求ID
    uint32_t requestId = requestIdCounter_.fetch_add(1);
    
    // 创建Promise和Future
    auto promise = std::make_shared<std::promise<ServerResponse>>();
    auto future = promise->get_future();
    
    try {
        std::cout << "准备发送请求:" << std::endl;
        std::cout << "- 类型: " << request.type() << std::endl;
        std::cout << "- 请求ID: " << requestId << std::endl;
        
        // 设置请求ID
        ControlMessage requestWithId = request;
        requestWithId.set_request_id(requestId);
        requestWithId.set_user_id(userId_);  // 确保设置了用户ID
        
        // 序列化控制消息
        std::vector<uint8_t> messageData(requestWithId.ByteSizeLong());
        if (!requestWithId.SerializeToArray(messageData.data(), static_cast<int>(messageData.size()))) {
            throw std::runtime_error("序列化控制消息失败");
        }
        
        std::cout << "- 序列化后大小: " << messageData.size() << " 字节" << std::endl;
        std::cout << "- 消息内容（十六进制）: ";
        for (const auto& byte : messageData) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
        std::cout << std::dec << std::endl;
        
        // 存储Promise（在发送之前）
        {
            std::lock_guard<std::mutex> lock(responseMutex_);
            pendingPromises_[requestId] = promise;
            std::cout << "已存储Promise，请求ID: " << requestId << std::endl;
            std::cout << "当前等待中的Promise数量: " << pendingPromises_.size() << std::endl;
        }
        
        // 发送请求
        if (!connection_->send(messageData)) {
            std::cout << "发送请求失败" << std::endl;
            return false;
        }
        std::cout << "请求已发送，等待响应..." << std::endl;
        
        // 等待响应（设置5秒超时）
        auto status = future.wait_for(std::chrono::seconds(5));
        if (status == std::future_status::ready) {
            response = future.get();
            std::cout << "成功接收到响应:" << std::endl;
            std::cout << "- 状态: " << response.status() << std::endl;
            std::cout << "- 消息: " << response.message() << std::endl;
            std::cout << "- 请求ID: " << response.request_id() << std::endl;
            return true;
        }
        std::cout << "等待响应超时（5秒）" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "发送请求时发生错误: " << e.what() << std::endl;
    }
    
    // 清理Promise
    {
        std::lock_guard<std::mutex> lock(responseMutex_);
        pendingPromises_.erase(requestId);
        std::cout << "已清理Promise，请求ID: " << requestId << std::endl;
    }
    
    return false;
}

} // namespace voicechat 