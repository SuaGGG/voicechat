#pragma once

#include "network_interface.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include <mutex>

namespace voicechat {

class AsioConnection : public INetworkConnection {
public:
    AsioConnection();
    ~AsioConnection() override;

    bool connect(const std::string& host, uint16_t port) override;
    void disconnect() override;
    bool send(const std::vector<uint8_t>& data) override;
    bool isConnected() const override;

    void setMessageCallback(MessageCallback callback) override;
    void setErrorCallback(ErrorCallback callback) override;
    void setConnectedCallback(ConnectionCallback callback) override;
    void setDisconnectedCallback(ConnectionCallback callback) override;

private:
    void doConnect(const boost::asio::ip::tcp::endpoint& endpoint);
    void doRead();
    void doWrite();
    void handleError(const boost::system::error_code& error);

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::socket socket_;
    std::thread io_thread_;
    
    MessageCallback messageCallback_;
    ErrorCallback errorCallback_;
    ConnectionCallback connectedCallback_;
    ConnectionCallback disconnectedCallback_;
    
    std::mutex mutex_;
    std::queue<std::vector<uint8_t>> writeQueue_;
    bool isWriting_;
    
    // 用于读取的缓冲区
    static constexpr size_t HEADER_SIZE = 4;
    std::vector<uint8_t> readBuffer_;
    std::vector<uint8_t> headerBuffer_;
    bool isConnected_;
};

class AsioServer : public INetworkServer {
public:
    AsioServer();
    ~AsioServer() override;

    bool start(uint16_t port) override;
    void stop() override;
    void broadcast(const std::vector<uint8_t>& data) override;
    bool sendTo(const std::string& clientId, const std::vector<uint8_t>& data) override;

    void setClientConnectedCallback(std::function<void(const std::string&)> callback) override;
    void setClientDisconnectedCallback(std::function<void(const std::string&)> callback) override;
    void setMessageCallback(std::function<void(const std::string&, const std::vector<uint8_t>&)> callback) override;

private:
    void doAccept();
    void removeClient(const std::string& clientId);
    void handleClientData(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                         const std::string& clientId,
                         std::shared_ptr<std::vector<uint8_t>> headerBuffer);

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::thread io_thread_;
    
    std::function<void(const std::string&)> clientConnectedCallback_;
    std::function<void(const std::string&)> clientDisconnectedCallback_;
    std::function<void(const std::string&, const std::vector<uint8_t>&)> messageCallback_;
    
    std::mutex clientsMutex_;
    std::unordered_map<std::string, std::shared_ptr<boost::asio::ip::tcp::socket>> clients_;
    bool running_;
};

} // namespace voicechat 