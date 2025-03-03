#include "voice_client.hpp"
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <sstream>
#include <iomanip>

namespace voicechat {

VoiceClient::VoiceClient(const std::string& userId)
    : userId_(userId)
    , muted_(false)
    , running_(false)
{
    // 初始化音频设备（作为输入设备）
    audioDevice_ = std::make_unique<PortAudioDevice>(true);
    audioDevice_->setCallback([this](const std::vector<float>& data) {
        // 将float数据转换为uint8_t
        std::vector<uint8_t> audioData(data.size() * sizeof(float));
        std::memcpy(audioData.data(), data.data(), audioData.size());
        onAudioData(audioData);
    });

    // 初始化音频编解码器
    audioCodec_ = std::make_unique<OpusCodec>();
    audioCodec_->initialize(48000, 1); // 48kHz, 单声道
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

        // 发送连接请求
        ControlMessage msg;
        msg.set_type(ControlMessage::JOIN);
        msg.set_user_id(userId_);
        
        std::vector<uint8_t> data(msg.ByteSizeLong());
        msg.SerializeToArray(data.data(), data.size());
        connection_->send(data);

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
        std::cerr << "Not connected to server" << std::endl;
        return false;
    }

    try {
        ControlMessage msg;
        msg.set_type(ControlMessage::JOIN);
        msg.set_user_id(userId_);
        msg.set_room_id(roomId);
        
        std::vector<uint8_t> data(msg.ByteSizeLong());
        msg.SerializeToArray(data.data(), data.size());
        connection_->send(data);

        currentRoomId_ = roomId;
        
        // 启动音频设备
        if (!audioDevice_->start()) {
            std::cerr << "Failed to start audio device" << std::endl;
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in joinRoom: " << e.what() << std::endl;
        return false;
    }
}

bool VoiceClient::leaveRoom() {
    if (!connection_ || currentRoomId_.empty()) {
        return false;
    }

    try {
        ControlMessage msg;
        msg.set_type(ControlMessage::LEAVE);
        msg.set_user_id(userId_);
        msg.set_room_id(currentRoomId_);
        
        std::vector<uint8_t> data(msg.ByteSizeLong());
        msg.SerializeToArray(data.data(), data.size());
        connection_->send(data);

        currentRoomId_.clear();
        
        // 停止音频设备
        audioDevice_->stop();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in leaveRoom: " << e.what() << std::endl;
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
        std::cout << "收到服务器消息，大小: " << data.size() << " 字节" << std::endl;
        
        // 检查消息大小是否至少包含头部
        if (data.size() < 4) {
            std::cerr << "消息太短，无法包含头部" << std::endl;
            return;
        }
        
        // 解析消息长度（4字节，小端序）
        uint32_t messageSize = 0;
        for (int i = 0; i < 4; ++i) {
            messageSize |= (static_cast<uint32_t>(data[i]) << (i * 8));
        }
        
        std::cout << "消息头指示的数据大小: " << messageSize << " 字节" << std::endl;
        
        // 检查消息大小是否合理
        if (data.size() != messageSize + 4) {
            std::cerr << "消息大小不匹配，期望: " << (messageSize + 4) << "，实际: " << data.size() << std::endl;
            return;
        }
        
        // 获取实际消息数据
        std::vector<uint8_t> messageData(data.begin() + 4, data.end());
        
        // 尝试解析为服务器响应
        ServerResponse response;
        if (response.ParseFromArray(messageData.data(), static_cast<int>(messageData.size()))) {
            std::cout << "成功解析为服务器响应消息" << std::endl;
            std::cout << "状态: " << (response.status() == ServerResponse::SUCCESS ? "成功" : "失败") << std::endl;
            std::cout << "消息内容: " << response.message() << std::endl;
            handleServerResponse(response);
            return;
        }
        std::cout << "不是服务器响应消息，尝试解析为音频消息" << std::endl;

        // 尝试解析为音频数据
        AudioData audioData;
        if (audioData.ParseFromArray(messageData.data(), static_cast<int>(messageData.size()))) {
            std::cout << "成功解析为音频消息，来自用户: " << audioData.user_id() << std::endl;
            handleAudioData(audioData);
            return;
        }

        // 如果都解析失败，打印消息内容以便调试
        std::cout << "无法解析消息内容，十六进制dump: ";
        for (const auto& byte : messageData) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
        std::cout << std::dec << std::endl;
        
        std::cerr << "无法解析服务器消息类型" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "处理服务器消息时发生错误: " << e.what() << std::endl;
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
    if (responsePromise_) {
        responsePromise_->set_value(response);
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
        
        // 准备完整的消息（4字节头 + 数据）
        std::vector<uint8_t> packet;
        packet.reserve(4 + messageData.size());
        
        // 添加长度头（4字节，小端序）
        uint32_t size = static_cast<uint32_t>(messageData.size());
        for (int i = 0; i < 4; ++i) {
            packet.push_back(static_cast<uint8_t>((size >> (i * 8)) & 0xFF));
        }
        
        // 添加数据
        packet.insert(packet.end(), messageData.begin(), messageData.end());
        
        // 发送数据
        connection_->send(packet);
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
            std::cout << "收到服务器响应: " << response.message() << std::endl;
            
            // 解析响应消息中的房间信息
            std::istringstream iss(response.message());
            std::string line;
            while (std::getline(iss, line)) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string roomId = line.substr(0, pos);
                    size_t count = std::stoull(line.substr(pos + 1));
                    rooms[roomId] = count;
                    std::cout << "解析到房间: " << roomId << " (在线人数: " << count << ")" << std::endl;
                }
            }
        } else {
            std::cout << "获取房间列表请求超时或失败" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "获取房间列表失败: " << e.what() << std::endl;
    }
    
    return rooms;
}

bool VoiceClient::sendRequest(const ControlMessage& request, ServerResponse& response) {
    if (!connection_) {
        throw std::runtime_error("未连接到服务器");
    }

    try {
        std::cout << "准备发送请求，类型: " << request.type() << std::endl;
        
        // 序列化控制消息
        std::vector<uint8_t> messageData(request.ByteSizeLong());
        if (!request.SerializeToArray(messageData.data(), static_cast<int>(messageData.size()))) {
            throw std::runtime_error("序列化控制消息失败");
        }
        
        // 准备完整的消息（4字节头 + 数据）
        std::vector<uint8_t> packet;
        packet.reserve(4 + messageData.size());
        
        // 添加长度头（4字节，小端序）
        uint32_t size = static_cast<uint32_t>(messageData.size());
        for (int i = 0; i < 4; ++i) {
            packet.push_back(static_cast<uint8_t>((size >> (i * 8)) & 0xFF));
        }
        
        // 添加数据
        packet.insert(packet.end(), messageData.begin(), messageData.end());
        
        // 设置新的 Promise
        {
            std::unique_lock<std::mutex> lock(responseMutex_);
            responsePromise_ = std::make_shared<std::promise<ServerResponse>>();
        }
        auto future = responsePromise_->get_future();
        
        // 发送请求
        if (!connection_->send(packet)) {
            std::cout << "发送请求失败" << std::endl;
            return false;
        }
        std::cout << "请求已发送，数据大小: " << messageData.size() << " 字节，等待响应..." << std::endl;
        
        // 等待响应（设置5秒超时）
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
            response = future.get();
            std::cout << "成功接收到响应: " << response.message() << std::endl;
            return true;
        }
        std::cout << "等待响应超时（5秒）" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "发送请求时发生错误: " << e.what() << std::endl;
    }
    
    return false;
}

} // namespace voicechat 