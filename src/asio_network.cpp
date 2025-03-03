#include "asio_network.hpp"
#include <iostream>

namespace voicechat {

// AsioConnection实现
AsioConnection::AsioConnection()
  : socket_(io_context_)
  , isWriting_(false)
  , isConnected_(false)
  , headerBuffer_(HEADER_SIZE)
{
}

AsioConnection::~AsioConnection() {
  disconnect();
}

bool AsioConnection::connect(const std::string& host, uint16_t port) {
  try {
    boost::asio::ip::tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    
    doConnect(*endpoints.begin());
    
    // 启动IO线程
    io_thread_ = std::thread([this]() {
      io_context_.run();
    });
    
    return true;
  } catch (const std::exception& e) {
    if (errorCallback_) {
      errorCallback_(e.what());
    }
    return false;
  }
}

void AsioConnection::disconnect() {
  if (isConnected_) {
    boost::system::error_code ec;
    socket_.close(ec);
    isConnected_ = false;
  }
  
  if (io_thread_.joinable()) {
    io_context_.stop();
    io_thread_.join();
  }
}

bool AsioConnection::send(const std::vector<uint8_t>& data) {
  if (!isConnected_) return false;
  
  // 准备数据包（4字节头 + 数据）
  std::vector<uint8_t> packet;
  packet.reserve(HEADER_SIZE + data.size());
  
  // 添加长度头
  uint32_t size = static_cast<uint32_t>(data.size());
  for (int i = 0; i < HEADER_SIZE; ++i) {
    packet.push_back(static_cast<uint8_t>((size >> (8 * i)) & 0xFF));
  }
  
  // 添加数据
  packet.insert(packet.end(), data.begin(), data.end());
  
  // 加入发送队列
  {
    std::lock_guard<std::mutex> lock(mutex_);
    writeQueue_.push(std::move(packet));
  }
  
  // 如果没有正在进行的写操作，启动一个
  if (!isWriting_) {
    doWrite();
  }
  
  return true;
}

bool AsioConnection::isConnected() const {
  return isConnected_;
}

void AsioConnection::setMessageCallback(MessageCallback callback) {
  messageCallback_ = std::move(callback);
}

void AsioConnection::setErrorCallback(ErrorCallback callback) {
  errorCallback_ = std::move(callback);
}

void AsioConnection::setConnectedCallback(ConnectionCallback callback) {
  connectedCallback_ = std::move(callback);
}

void AsioConnection::setDisconnectedCallback(ConnectionCallback callback) {
  disconnectedCallback_ = std::move(callback);
}

void AsioConnection::doConnect(const boost::asio::ip::tcp::endpoint& endpoint) {
  socket_.async_connect(endpoint,
    [this](const boost::system::error_code& error) {
      if (!error) {
        isConnected_ = true;
        if (connectedCallback_) {
          connectedCallback_();
        }
        doRead();
      } else {
        handleError(error);
      }
    });
}

void AsioConnection::doRead() {
  // 首先读取头部
  boost::asio::async_read(socket_,
    boost::asio::buffer(headerBuffer_),
    [this](const boost::system::error_code& error, std::size_t /*length*/) {
      if (!error) {
        // 解析数据长度
        uint32_t dataSize = 0;
        for (int i = 0; i < HEADER_SIZE; ++i) {
          dataSize |= (headerBuffer_[i] << (8 * i));
        }
        
        // 准备接收数据
        readBuffer_.resize(dataSize);
        boost::asio::async_read(socket_,
          boost::asio::buffer(readBuffer_),
          [this](const boost::system::error_code& error, std::size_t /*length*/) {
            if (!error) {
              if (messageCallback_) {
                messageCallback_(readBuffer_);
              }
              doRead(); // 继续读取下一个消息
            } else {
              handleError(error);
            }
          });
      } else {
        handleError(error);
      }
    });
}

void AsioConnection::doWrite() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (writeQueue_.empty()) {
    isWriting_ = false;
    return;
  }
  
  isWriting_ = true;
  boost::asio::async_write(socket_,
    boost::asio::buffer(writeQueue_.front()),
    [this](const boost::system::error_code& error, std::size_t /*length*/) {
      if (!error) {
        std::lock_guard<std::mutex> lock(mutex_);
        writeQueue_.pop();
        doWrite(); // 继续发送下一个消息
      } else {
        handleError(error);
      }
    });
}

void AsioConnection::handleError(const boost::system::error_code& error) {
  if (errorCallback_) {
    errorCallback_(error.message());
  }
  
  if (isConnected_) {
    isConnected_ = false;
    if (disconnectedCallback_) {
      disconnectedCallback_();
    }
  }
}

// AsioServer实现
AsioServer::AsioServer()
  : acceptor_(io_context_)
  , running_(false)
{
}

AsioServer::~AsioServer() {
  stop();
}

bool AsioServer::start(uint16_t port) {
  try {
    acceptor_.open(boost::asio::ip::tcp::v4());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    acceptor_.listen();
    
    running_ = true;
    doAccept();
    
    // 启动IO线程
    io_thread_ = std::thread([this]() {
      io_context_.run();
    });
    
    return true;
  } catch (const std::exception& e) {
    std::cerr << "Server start error: " << e.what() << std::endl;
    return false;
  }
}

void AsioServer::stop() {
  running_ = false;
  acceptor_.close();
  
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    for (auto& client : clients_) {
      client.second->close();
    }
    clients_.clear();
  }
  
  if (io_thread_.joinable()) {
    io_context_.stop();
    io_thread_.join();
  }
}

void AsioServer::broadcast(const std::vector<uint8_t>& data) {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  for (const auto& client : clients_) {
    sendTo(client.first, data);
  }
}

bool AsioServer::sendTo(const std::string& clientId, const std::vector<uint8_t>& data) {
  std::shared_ptr<boost::asio::ip::tcp::socket> clientSocket;
  {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    auto it = clients_.find(clientId);
    if (it == clients_.end()) {
      return false;
    }
    clientSocket = it->second;
  }
  
  // 准备数据包
  std::vector<uint8_t> packet;
  packet.reserve(4 + data.size());
  
  // 添加长度头
  uint32_t size = static_cast<uint32_t>(data.size());
  for (int i = 0; i < 4; ++i) {
    packet.push_back(static_cast<uint8_t>((size >> (8 * i)) & 0xFF));
  }
  
  // 添加数据
  packet.insert(packet.end(), data.begin(), data.end());
  
  // 异步发送
  boost::asio::async_write(*clientSocket,
    boost::asio::buffer(packet),
    [this, clientId](const boost::system::error_code& error, std::size_t /*length*/) {
      if (error) {
        removeClient(clientId);
      }
    });
  
  return true;
}

void AsioServer::setClientConnectedCallback(std::function<void(const std::string&)> callback) {
  clientConnectedCallback_ = std::move(callback);
}

void AsioServer::setClientDisconnectedCallback(std::function<void(const std::string&)> callback) {
  clientDisconnectedCallback_ = std::move(callback);
}

void AsioServer::setMessageCallback(std::function<void(const std::string&, const std::vector<uint8_t>&)> callback) {
  messageCallback_ = std::move(callback);
}

void AsioServer::doAccept() {
  if (!running_) return;
  
  auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
  acceptor_.async_accept(*socket,
    [this, socket](const boost::system::error_code& error) {
      if (!error) {
        // 生成客户端ID
        std::string clientId = std::to_string(reinterpret_cast<uintptr_t>(socket.get()));
        
        // 添加到客户端列表
        {
          std::lock_guard<std::mutex> lock(clientsMutex_);
          clients_[clientId] = socket;
        }
        
        if (clientConnectedCallback_) {
          clientConnectedCallback_(clientId);
        }
        
        // 开始接收数据
        auto headerBuffer = std::make_shared<std::vector<uint8_t>>(4);
        boost::asio::async_read(*socket,
          boost::asio::buffer(*headerBuffer),
          [this, socket, clientId, headerBuffer](const boost::system::error_code& error, std::size_t /*length*/) {
            if (!error) {
              handleClientData(socket, clientId, headerBuffer);
            } else {
              removeClient(clientId);
            }
          });
      }
      
      doAccept(); // 继续接受新的连接
    });
}

void AsioServer::removeClient(const std::string& clientId) {
  std::lock_guard<std::mutex> lock(clientsMutex_);
  auto it = clients_.find(clientId);
  if (it != clients_.end()) {
    it->second->close();
    clients_.erase(it);
    
    if (clientDisconnectedCallback_) {
      clientDisconnectedCallback_(clientId);
    }
  }
}

void AsioServer::handleClientData(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                               const std::string& clientId,
                               std::shared_ptr<std::vector<uint8_t>> headerBuffer) {
  // 解析数据长度
  uint32_t dataSize = 0;
  for (int i = 0; i < 4; ++i) {
    dataSize |= ((*headerBuffer)[i] << (8 * i));
  }
  
  // 准备接收数据
  auto dataBuffer = std::make_shared<std::vector<uint8_t>>(dataSize);
  boost::asio::async_read(*socket,
    boost::asio::buffer(*dataBuffer),
    [this, socket, clientId, dataBuffer, headerBuffer](const boost::system::error_code& error, std::size_t /*length*/) {
      if (!error) {
        if (messageCallback_) {
          messageCallback_(clientId, *dataBuffer);
        }
        
        // 继续读取下一个消息的头部
        boost::asio::async_read(*socket,
          boost::asio::buffer(*headerBuffer),
          [this, socket, clientId, headerBuffer](const boost::system::error_code& error, std::size_t /*length*/) {
            if (!error) {
              handleClientData(socket, clientId, headerBuffer);
            } else {
              removeClient(clientId);
            }
          });
      } else {
        removeClient(clientId);
      }
    });
}

} // namespace voicechat 