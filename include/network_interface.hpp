#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace voicechat {

// 网络事件回调类型定义
using MessageCallback = std::function<void(const std::vector<uint8_t>&)>;
using ErrorCallback = std::function<void(const std::string&)>;
using ConnectionCallback = std::function<void()>;

// 网络连接接口
class INetworkConnection {
public:
    virtual ~INetworkConnection() = default;
    
    // 连接到服务器
    virtual bool connect(const std::string& host, uint16_t port) = 0;
    
    // 断开连接
    virtual void disconnect() = 0;
    
    // 发送数据
    virtual bool send(const std::vector<uint8_t>& data) = 0;
    
    // 设置回调
    virtual void setMessageCallback(MessageCallback callback) = 0;
    virtual void setErrorCallback(ErrorCallback callback) = 0;
    virtual void setConnectedCallback(ConnectionCallback callback) = 0;
    virtual void setDisconnectedCallback(ConnectionCallback callback) = 0;
    
    // 检查连接状态
    virtual bool isConnected() const = 0;
};

// 网络服务器接口
class INetworkServer {
public:
    virtual ~INetworkServer() = default;
    
    // 启动服务器
    virtual bool start(uint16_t port) = 0;
    
    // 停止服务器
    virtual void stop() = 0;
    
    // 广播消息给所有客户端
    virtual void broadcast(const std::vector<uint8_t>& data) = 0;
    
    // 发送消息给特定客户端
    virtual bool sendTo(const std::string& clientId, const std::vector<uint8_t>& data) = 0;
    
    // 设置回调
    virtual void setClientConnectedCallback(std::function<void(const std::string&)> callback) = 0;
    virtual void setClientDisconnectedCallback(std::function<void(const std::string&)> callback) = 0;
    virtual void setMessageCallback(std::function<void(const std::string&, const std::vector<uint8_t>&)> callback) = 0;
};

} // namespace voicechat 