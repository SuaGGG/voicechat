#include "voice_client.hpp"
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <thread>
#include <chrono>
#include <future>
#include <optional>
#include <vector>
#include <mutex>
#include <memory>
#include <cmath>
namespace voicechat {

VoiceClient::VoiceClient(const std::string& userId)
    : userId_(userId)
    , muted_(false)
    , running_(false)
    , useVirtualAudio_(false)
{
    try {
        std::cout << "初始化音频设备..." << std::endl;
        
        // 初始化音频设备
        if (!initAudio()) {
            std::cerr << "警告: 音频设备初始化失败，将使用虚拟音频模式" << std::endl;
            useVirtualAudio_ = true;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "初始化音频设备时发生错误: " << e.what() << std::endl;
        std::cerr << "将使用虚拟音频模式" << std::endl;
        useVirtualAudio_ = true;
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
            // 如果已加入房间，先尝试离开
            if (currentRoomId_.has_value()) {
                ControlMessage msg;
                msg.set_type(ControlMessage::LEAVE);
                msg.set_user_id(userId_);
                
                std::vector<uint8_t> data(msg.ByteSizeLong());
                msg.SerializeToArray(data.data(), static_cast<int>(data.size()));
                connection_->send(data);
                
                std::cout << "已发送离开房间请求" << std::endl;
                
                // 等待一小段时间确保消息发送
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
            connection_->disconnect();
        } catch (const std::exception& e) {
            std::cerr << "断开连接时发生错误: " << e.what() << std::endl;
        }
        
        connection_.reset();
    }
    
    if (audioDevice_) {
        try {
            audioDevice_->stop();
        } catch (const std::exception& e) {
            std::cerr << "停止音频设备时发生错误: " << e.what() << std::endl;
        }
    }
    
    currentRoomId_.reset();
    running_ = false;
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
        
        // 发送请求并等待响应
        ServerResponse response;
        if (!sendRequest(msg, response)) {
            std::cerr << "发送加入房间请求失败或超时" << std::endl;
            return false;
        }
        
        if (response.status() != ServerResponse::SUCCESS) {
            std::cerr << "加入房间失败: " << response.message() << std::endl;
            return false;
        }
        
        currentRoomId_ = roomId;
        std::cout << "成功加入房间: " << roomId << std::endl;
        
        // 启动音频设备
        if (audioDevice_) {
            if (!audioDevice_->start()) {
                std::cerr << "警告: 启动音频设备失败，但已加入房间" << std::endl;
                // 不要因为音频设备失败就退出房间
            } else {
                std::cout << "音频设备已启动" << std::endl;
                
                // 如果是虚拟模式，启动一个线程定期发送测试音频数据
                if (useVirtualAudio_) {
                    std::cout << "虚拟音频模式: 启动测试音频发送线程" << std::endl;
                    
                    // 创建并启动测试音频线程
                    std::thread([this, roomId]() {
                        while (running_ && currentRoomId_.has_value() && currentRoomId_.value() == roomId) {
                            if (!muted_) {
                                // 生成测试音频数据
                                std::vector<float> testData(1024);
                                static float phase = 0.0f;
                                for (size_t i = 0; i < testData.size(); ++i) {
                                    testData[i] = 0.8f * std::sin(phase);
                                    phase += 0.1f;
                                    if (phase > 2 * M_PI) phase -= 2 * M_PI;
                                }
                                
                                // 创建音频消息
                                AudioData audioMsg;
                                audioMsg.set_user_id(userId_);
                                audioMsg.set_audio_payload(std::string(
                                    reinterpret_cast<const char*>(testData.data()), 
                                    testData.size() * sizeof(float)
                                ));
                                audioMsg.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
                                audioMsg.set_sequence_number(0);
                                
                                // 序列化并发送
                                std::vector<uint8_t> serialized(audioMsg.ByteSizeLong());
                                audioMsg.SerializeToArray(serialized.data(), static_cast<int>(serialized.size()));
                                connection_->send(serialized);
                                
                                std::cout << "发送虚拟测试音频数据，大小: " << testData.size() << " 样本" << std::endl;
                            }
                            
                            // 每秒发送一次测试数据
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                        }
                        
                        std::cout << "测试音频线程结束" << std::endl;
                    }).detach();
                }
            }
        } else {
            std::cerr << "警告: 音频设备未初始化，无法启动" << std::endl;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "加入房间时发生错误: " << e.what() << std::endl;
        return false;
    }
}

bool VoiceClient::leaveRoom() {
    if (!connection_ || !currentRoomId_.has_value()) {
        std::cerr << "未连接到服务器或未加入房间" << std::endl;
        return false;
    }

    try {
        std::cout << "尝试离开房间: " << currentRoomId_.value() << std::endl;
        
        // 创建控制消息
        ControlMessage msg;
        msg.set_type(ControlMessage::LEAVE);
        msg.set_user_id(userId_);
        
        // 发送请求并等待响应
        ServerResponse response;
        if (!sendRequest(msg, response)) {
            std::cerr << "发送离开房间请求失败或超时" << std::endl;
            return false;
        }
        
        if (response.status() != ServerResponse::SUCCESS) {
            std::cerr << "离开房间失败: " << response.message() << std::endl;
            return false;
        }
        
        // 停止音频设备（如果不是虚拟模式）
        if (!useVirtualAudio_ && audioDevice_) {
            audioDevice_->stop();
            std::cout << "音频设备已停止" << std::endl;
        } else if (useVirtualAudio_) {
            std::cout << "虚拟音频模式: 不需要停止实际音频设备" << std::endl;
        }
        
        std::string oldRoom = currentRoomId_.value();
        currentRoomId_.reset();
        std::cout << "已离开房间: " << oldRoom << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "离开房间时发生错误: " << e.what() << std::endl;
        return false;
    }
}

void VoiceClient::setMuted(bool muted) {
    muted_ = muted;
    std::cout << "麦克风状态: " << (muted_ ? "已静音" : "已取消静音") << std::endl;
}

bool VoiceClient::isMuted() const {
    return muted_;
}

void VoiceClient::onMessage(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        std::cerr << "收到空消息" << std::endl;
        return;
    }

    try {
        // 尝试解析为 ServerResponse
        ServerResponse response;
        bool parseSuccess = response.ParseFromArray(data.data(), static_cast<int>(data.size()));
        
        if (parseSuccess) {
            std::cout << "收到服务器响应:" << std::endl;
            std::cout << "- 状态: " << (response.status() == ServerResponse::SUCCESS ? "成功" : "失败") << std::endl;
            std::cout << "- 消息: " << response.message() << std::endl;
            std::cout << "- 请求ID: " << response.request_id() << std::endl;
            
            // 检查是否有等待此响应的 Promise
            uint32_t requestId = response.request_id();
            std::shared_ptr<std::promise<ServerResponse>> promise;
            
            {
                std::lock_guard<std::mutex> lock(responseMutex_);
                auto it = pendingPromises_.find(requestId);
                if (it != pendingPromises_.end()) {
                    promise = it->second;
                } else {
                    std::cout << "未找到对应的Promise，请求ID: " << requestId << std::endl;
                    std::cout << "当前等待中的Promise数量: " << pendingPromises_.size() << std::endl;
                }
            }
            
            // 如果找到了 Promise，设置值
            if (promise) {
                promise->set_value(response);
            }
            
            return;
        }
        
        // 如果不是 ServerResponse，尝试解析为 AudioData
        AudioData audioData;
        parseSuccess = audioData.ParseFromArray(data.data(), static_cast<int>(data.size()));
        
        if (parseSuccess) {
            std::cout << "收到音频数据，来自用户: " << audioData.user_id() << std::endl;
            handleAudioData(audioData);
            return;
        }
        
        // 如果都不是，打印错误
        std::cerr << "无法解析收到的消息，大小: " << data.size() << " 字节" << std::endl;
        std::cout << "消息内容（十六进制）: ";
        for (size_t i = 0; i < std::min(data.size(), size_t(32)); ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
        }
        if (data.size() > 32) {
            std::cout << "...";
        }
        std::cout << std::dec << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "处理消息时发生异常: " << e.what() << std::endl;
    }
}

void VoiceClient::handleAudioData(const AudioData& audioData) {
    if (audioData.user_id() == userId_) {
        return; // 忽略自己的音频
    }
    
    try {
        // // 如果是虚拟音频模式，只打印日志但不处理音频
        // if (useVirtualAudio_) {
        //     std::cout << "收到来自 " << audioData.user_id() << " 的音频数据 (虚拟模式，不播放)" << std::endl;
        //     return;
        // }
        
        // 获取音频数据
        const std::string& audioPayload = audioData.audio_payload();
        if (audioPayload.empty()) {
            std::cout << "收到空的音频数据" << std::endl;
            return;
        }
        
        std::cout << "收到来自 " << audioData.user_id() << " 的音频数据，大小: " 
                  << audioPayload.size() << " 字节" << std::endl;
        
        // 将字符串数据转换回 float 数组
        const float* floatData = reinterpret_cast<const float*>(audioPayload.data());
        size_t floatCount = audioPayload.size() / sizeof(float);
        
        // 创建 float 向量
        std::vector<float> samples(floatData, floatData + floatCount);
        
        // 打印一些样本值，检查数据是否正确
        std::cout << "音频样本值 (前10个): ";
        for (size_t i = 0; i < std::min(samples.size(), size_t(10)); ++i) {
            std::cout << samples[i] << " ";
        }
        std::cout << std::endl;
        
        // 检查音频样本的范围
        float minSample = 0.0f, maxSample = 0.0f;
        if (!samples.empty()) {
            minSample = maxSample = samples[0];
            for (const auto& sample : samples) {
                minSample = std::min(minSample, sample);
                maxSample = std::max(maxSample, sample);
            }
        }
        std::cout << "音频样本范围: [" << minSample << ", " << maxSample << "]" << std::endl;
        
        // 如果样本值太小，可能是音量问题，尝试放大
        if (maxSample < 0.1f && minSample > -0.1f) {
            std::cout << "音频样本值较小，尝试放大 10 倍" << std::endl;
            for (auto& sample : samples) {
                sample *= 10.0f;
            }
        }
        
        // 使用智能指针管理输出设备，确保资源正确释放
        static std::shared_ptr<PortAudioDevice> outputDevice;
        static bool outputDeviceInitialized = false;
        
        // 只在第一次或设备未初始化时创建和初始化设备
        if (!outputDevice) {
            try {
                outputDevice = std::make_shared<PortAudioDevice>(false); // false 表示输出设备
                std::cout << "创建音频输出设备" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "创建音频输出设备失败: " << e.what() << std::endl;
                return;
            }
        }
        
        // 初始化设备（如果尚未初始化）
        if (!outputDeviceInitialized) {
            if (outputDevice->isInitialized() && outputDevice->initialize(48000, 1)) {
                std::cout << "成功初始化音频输出设备" << std::endl;
                
                // 设置回调函数，但我们不需要在这里处理输入
                outputDevice->setCallback([](const std::vector<float>& data) {
                    // 输出设备不需要处理输入
                });
                
                // 启动输出设备
                if (outputDevice->start()) {
                    std::cout << "音频输出设备已启动" << std::endl;
                    outputDeviceInitialized = true;
                } else {
                    std::cerr << "启动音频输出设备失败" << std::endl;
                    return;
                }
            } else {
                std::cerr << "初始化音频输出设备失败" << std::endl;
                return;
            }
        }
        
        // 使用设备播放音频
        if (outputDeviceInitialized) {
            if (outputDevice->play(samples)) {
                std::cout << "音频数据已发送到输出设备" << std::endl;
            } else {
                std::cerr << "无法播放音频数据" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "处理音频数据时发生异常: " << e.what() << std::endl;
    }
}

void VoiceClient::handleServerResponse(const ServerResponse& response) {
    std::lock_guard<std::mutex> lock(responseMutex_);
    if (pendingPromises_[response.request_id()]) {
        pendingPromises_[response.request_id()]->set_value(response);
    }
}

void VoiceClient::onAudioData(const std::vector<uint8_t>& audioData) {
    if (!connection_ || !currentRoomId_.has_value() || muted_) {
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

std::unordered_map<std::string, std::vector<std::string> > VoiceClient::getAvailableRooms() {
    
    std::unordered_map<std::string, std::vector<std::string> > rooms;

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
                    size_t posRoomId = room.find(':');
                    if (posRoomId == std::string::npos) {
                        std::cerr << "无效的房间信息: " << room << std::endl;
                        continue;
                    }
                    std::string roomId = room.substr(0, posRoomId);
                    std::string countStr = room.substr(posRoomId + 1);
                    std::vector<std::string> participants;
                    
                    for (size_t i = 0, j = 0; i < countStr.size(); i ++ ) {
                      if (countStr[i] == ':') {
                        participants.push_back(countStr.substr(j, i - 1));
                        j = i + 1;
                      }
                      if (countStr[i] == '.') {
                        break;
                      }
                    }
                    try {
                      rooms[roomId] = participants;
                      std::cout << "解析到房间: " << roomId << " (在线人数: " << participants.size() << ")" << std::endl;
                    } catch (const std::exception& e) {
                      std::cerr << "解析房间人数失败: " << countStr << ": " << e.what() << std::endl;
                      continue;
                    }
                }
            } else std::cout << "获取房间列表失败: " << response.message() << std::endl;
            
        } else std::cout << "获取房间列表请求超时或失败" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "获取房间列表失败: " << e.what() << std::endl;
    }
  
    if (rooms.empty()) {
        std::cout << "当前没有可用的房间" << std::endl;
    } else {
      std::cout << "\n当前可用房间列表:" << std::endl;
      for (const auto& [roomId, participants] : rooms) {
        std::cout << "- 房间: " << roomId << " (在线人数: " << participants.size() << ")" << std::endl;
        std::cout << "  在线用户: ";
        for (const auto& participant : participants) std::cout << participant << " ";
        std::cout << std::endl;
      }
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

bool VoiceClient::initAudio() {
    try {
        // 初始化音频编解码器
        audioCodec_ = std::make_unique<OpusCodec>();
        if (!audioCodec_->initialize(48000, 1)) { // 48kHz, 单声道
            std::cerr << "初始化音频编解码器失败" << std::endl;
            useVirtualAudio_ = true;
        }
        
        // 尝试初始化实际音频设备
        bool useRealAudio = false;
        
        if (!useVirtualAudio_) {
            try {
                // 尝试创建实际音频设备
                auto realDevice = std::make_unique<PortAudioDevice>(true);
                
                // 检查设备是否成功初始化
                if (realDevice->initialize(48000, 1) && realDevice->isInitialized()) {
                    audioDevice_ = std::move(realDevice);
                    useRealAudio = true;
                    std::cout << "成功初始化实际音频设备" << std::endl;
                } else {
                    std::cerr << "实际音频设备初始化失败，将使用虚拟音频模式" << std::endl;
                    useVirtualAudio_ = true;
                }
            } catch (const std::exception& e) {
                std::cerr << "创建音频设备时发生异常: " << e.what() << std::endl;
                std::cerr << "将使用虚拟音频模式" << std::endl;
                useVirtualAudio_ = true;
            }
        }
        
        // // 如果需要使用虚拟音频设备
        // if (useVirtualAudio_) {
        //     audioDevice_ = std::make_unique<NullAudioDevice>(true);
        //     audioDevice_->initialize(48000, 1);
        //     std::cout << "已启用虚拟音频模式" << std::endl;
        // }
        
        // // 设置音频回调
        // audioDevice_->setCallback([this](const std::vector<float>& data) {
        //     if (data.empty() || muted_ || !currentRoomId_.has_value()) {
        //         return; // 如果没有数据、静音或未加入房间，不发送音频
        //     }
            
        //     if (useVirtualAudio_) {
        //         // 虚拟模式下不处理实际音频数据
        //         return;
        //     }
            
        //     try {
        //         // 在虚拟模式下，生成一些测试音频数据
        //         std::vector<float> testData;
        //         if (useVirtualAudio_) {
        //             // 生成一个简单的正弦波作为测试数据
        //             static float phase = 0.0f;
        //             testData.resize(1024);
        //             for (size_t i = 0; i < testData.size(); ++i) {
        //                 // 增加音量，使用 0.8 而不是 0.5
        //                 testData[i] = 0.8f * std::sin(phase);
        //                 phase += 0.1f;
        //                 if (phase > 2 * M_PI) phase -= 2 * M_PI;
        //             }
        //         }
                
        //         // 创建音频消息
        //         AudioData audioMsg;
        //         audioMsg.set_user_id(userId_);
                
        //         // 使用实际数据或测试数据
        //         const std::vector<float>& audioData = useVirtualAudio_ ? testData : data;
                
        //         // 将 float 数组转换为字符串
        //         audioMsg.set_audio_payload(std::string(
        //             reinterpret_cast<const char*>(audioData.data()), 
        //             audioData.size() * sizeof(float)
        //         ));
                
        //         audioMsg.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        //         audioMsg.set_sequence_number(0); // 使用固定序列号
                
        //         // 序列化并发送
        //         std::vector<uint8_t> serialized(audioMsg.ByteSizeLong());
        //         audioMsg.SerializeToArray(serialized.data(), static_cast<int>(serialized.size()));
        //         connection_->send(serialized);
                
        //         // 打印日志
        //         if (useVirtualAudio_) {
        //             std::cout << "发送虚拟音频数据，大小: " << audioData.size() << " 样本" << std::endl;
        //         } else {
        //             std::cout << "发送实际音频数据，大小: " << audioData.size() << " 样本" << std::endl;
        //         }
        //     } catch (const std::exception& e) {
        //         std::cerr << "处理音频数据时发生错误: " << e.what() << std::endl;
        //     }
        // });
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "初始化音频设备失败: " << e.what() << std::endl;
        std::cerr << "将使用虚拟音频模式" << std::endl;
        useVirtualAudio_ = true;
        
        // 创建虚拟音频设备作为后备
        audioDevice_ = std::make_unique<NullAudioDevice>(true);
        audioDevice_->initialize(48000, 1);
        
        return true; // 返回成功，但使用虚拟模式
    }
}

std::optional<std::string> VoiceClient::getCurrentRoomId() const {
    return currentRoomId_;
}

std::vector<AudioDeviceInfo> VoiceClient::getAudioInputDevices() const {
    if (useVirtualAudio_) {
        std::cout << "使用虚拟音频模式，返回虚拟输入设备" << std::endl;
        
        // 创建一个虚拟输入设备
        AudioDeviceInfo virtualDevice;
        virtualDevice.index = 0;
        virtualDevice.name = "虚拟麦克风";
        virtualDevice.maxInputChannels = 2;
        virtualDevice.maxOutputChannels = 0;
        virtualDevice.defaultSampleRate = 44100;
        
        return {virtualDevice};
    }
    
    try {
        auto devices = PortAudioDevice::getAvailableDevices(true);
        
        // 调试输出
        std::cout << "===== 音频输入设备列表 =====" << std::endl;
        std::cout << "找到 " << devices.size() << " 个音频输入设备" << std::endl;
        for (const auto& device : devices) {
            std::cout << "设备 #" << device.index << ": " << device.name 
                    << " (输入通道: " << device.maxInputChannels
                    << ", 输出通道: " << device.maxOutputChannels
                    << ", 采样率: " << device.defaultSampleRate << ")" << std::endl;
        }
        std::cout << "=========================" << std::endl;
        
        // 如果没有找到任何设备，添加一个虚拟设备
        if (devices.empty()) {
            std::cout << "没有找到真实的音频输入设备，添加虚拟设备" << std::endl;
            AudioDeviceInfo virtualDevice;
            virtualDevice.index = 0;
            virtualDevice.name = "虚拟麦克风";
            virtualDevice.maxInputChannels = 2;
            virtualDevice.maxOutputChannels = 0;
            virtualDevice.defaultSampleRate = 44100;
            devices.push_back(virtualDevice);
        }
        
        return devices;
    } catch (const std::exception& e) {
        std::cerr << "获取音频输入设备列表时发生错误: " << e.what() << std::endl;
        
        // 返回一个虚拟设备作为后备
        AudioDeviceInfo virtualDevice;
        virtualDevice.index = 0;
        virtualDevice.name = "虚拟麦克风";
        virtualDevice.maxInputChannels = 2;
        virtualDevice.maxOutputChannels = 0;
        virtualDevice.defaultSampleRate = 44100;
        return {virtualDevice};
    }
}

std::vector<AudioDeviceInfo> VoiceClient::getAudioOutputDevices() const {
    if (useVirtualAudio_) {
        std::cout << "使用虚拟音频模式，返回虚拟输出设备" << std::endl;
        
        // 创建一个虚拟输出设备
        AudioDeviceInfo virtualDevice;
        virtualDevice.index = 0;
        virtualDevice.name = "虚拟扬声器";
        virtualDevice.maxInputChannels = 0;
        virtualDevice.maxOutputChannels = 2;
        virtualDevice.defaultSampleRate = 44100;
        
        return {virtualDevice};
    }
    
    try {
        auto devices = PortAudioDevice::getAvailableDevices(false);
        
        // 调试输出
        std::cout << "===== 音频输出设备列表 =====" << std::endl;
        std::cout << "找到 " << devices.size() << " 个音频输出设备" << std::endl;
        for (const auto& device : devices) {
            std::cout << "设备 #" << device.index << ": " << device.name 
                    << " (输入通道: " << device.maxInputChannels
                    << ", 输出通道: " << device.maxOutputChannels
                    << ", 采样率: " << device.defaultSampleRate << ")" << std::endl;
        }
        std::cout << "=========================" << std::endl;
        
        // 如果没有找到任何设备，添加一个虚拟设备
        if (devices.empty()) {
            std::cout << "没有找到真实的音频输出设备，添加虚拟设备" << std::endl;
            AudioDeviceInfo virtualDevice;
            virtualDevice.index = 0;
            virtualDevice.name = "虚拟扬声器";
            virtualDevice.maxInputChannels = 0;
            virtualDevice.maxOutputChannels = 2;
            virtualDevice.defaultSampleRate = 44100;
            devices.push_back(virtualDevice);
        }
        
        return devices;
    } catch (const std::exception& e) {
        std::cerr << "获取音频输出设备列表时发生错误: " << e.what() << std::endl;
        
        // 返回一个虚拟设备作为后备
        AudioDeviceInfo virtualDevice;
        virtualDevice.index = 0;
        virtualDevice.name = "虚拟扬声器";
        virtualDevice.maxInputChannels = 0;
        virtualDevice.maxOutputChannels = 2;
        virtualDevice.defaultSampleRate = 44100;
        return {virtualDevice};
    }
}

bool VoiceClient::setAudioInputDevice(int deviceIndex) {
    if (useVirtualAudio_) {
        std::cout << "虚拟音频模式下设置输入设备索引: " << deviceIndex << std::endl;
        return true;  // 在虚拟模式下总是返回成功
    }
    auto inputDevice = dynamic_cast<PortAudioDevice*>(audioDevice_.get());
    if (!inputDevice) {
        return false;
    }
    return inputDevice->setDevice(deviceIndex);
}

bool VoiceClient::setAudioOutputDevice(int deviceIndex) {
    if (useVirtualAudio_) {
        std::cout << "虚拟音频模式下设置输出设备索引: " << deviceIndex << std::endl;
        return true;  // 在虚拟模式下总是返回成功
    }
    auto outputDevice = dynamic_cast<PortAudioDevice*>(audioDevice_.get());
    if (!outputDevice) {
        return false;
    }
    return outputDevice->setDevice(deviceIndex);
}

void VoiceClient::setInputVolume(float volume) {
    if (useVirtualAudio_) {
        std::cout << "虚拟音频模式下设置输入音量: " << volume << std::endl;
        return;  // 在虚拟模式下直接返回
    }
    if (audioDevice_) {
        audioDevice_->setVolume(std::clamp(volume, 0.0f, 1.0f));
    }
}

void VoiceClient::setOutputVolume(float volume) {
    if (useVirtualAudio_) {
        std::cout << "虚拟音频模式下设置输出音量: " << volume << std::endl;
        return;  // 在虚拟模式下直接返回
    }
    if (audioDevice_) {
        audioDevice_->setVolume(std::clamp(volume, 0.0f, 1.0f));
    }
}

float VoiceClient::getInputVolume() const {
    if (useVirtualAudio_) {
        return 1.0f;  // 在虚拟模式下返回默认音量
    }
    if (audioDevice_) {
        return audioDevice_->getVolume();
    }
    return 0.0f;
}

float VoiceClient::getOutputVolume() const {
    if (useVirtualAudio_) {
        return 1.0f;  // 在虚拟模式下返回默认音量
    }
    if (audioDevice_) {
        return audioDevice_->getVolume();
    }
    return 0.0f;
}

PortAudioDevice* VoiceClient::getInputDevice() const {
    return dynamic_cast<PortAudioDevice*>(audioDevice_.get());
}

PortAudioDevice* VoiceClient::getOutputDevice() const {
    return dynamic_cast<PortAudioDevice*>(audioDevice_.get());
}

} // namespace voicechat 