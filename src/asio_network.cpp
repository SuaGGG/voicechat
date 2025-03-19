#include "asio_network.hpp"
#include <iostream>
#include <iomanip>

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
  std::cout << "准备发送数据:" << std::endl;
  std::cout << "- 原始数据大小: " << size << " 字节" << std::endl;
  
  // 使用小端序添加长度头
  packet.push_back(static_cast<uint8_t>(size & 0xFF));
  packet.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
  packet.push_back(static_cast<uint8_t>((size >> 16) & 0xFF));
  packet.push_back(static_cast<uint8_t>((size >> 24) & 0xFF));
  
  // 添加数据
  packet.insert(packet.end(), data.begin(), data.end());
  
  std::cout << "- 完整数据包大小: " << packet.size() << " 字节" << std::endl;
  std::cout << "- 数据包内容（十六进制）: ";
  for (const auto& byte : packet) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
  }
  std::cout << std::dec << std::endl;
  
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
        // 解析数据长度（小端序）
        uint32_t dataSize = 0;
        dataSize = static_cast<uint32_t>(headerBuffer_[0]) |
                  (static_cast<uint32_t>(headerBuffer_[1]) << 8) |
                  (static_cast<uint32_t>(headerBuffer_[2]) << 16) |
                  (static_cast<uint32_t>(headerBuffer_[3]) << 24);
        // 准备接收数据
        readBuffer_.resize(dataSize);
        boost::asio::async_read(socket_,
          boost::asio::buffer(readBuffer_),
          [this](const boost::system::error_code& error, std::size_t length) {
            if (!error) {
              
              if (messageCallback_) {
                messageCallback_(readBuffer_);
              }
              doRead(); // 继续读取下一个消息
            } else {
              std::cerr << "读取消息体失败: " << error.message() << std::endl;
              handleError(error);
            }
          });
      } else {
        std::cerr << "读取消息头部失败: " << error.message() << std::endl;
        handleError(error);
      }
    });
}

void AsioConnection::doWrite() {
    std::vector<uint8_t> data;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (writeQueue_.empty()) {
            isWriting_ = false;
            return;
        }
        
        isWriting_ = true;
        data = std::move(writeQueue_.front());
        writeQueue_.pop();
    }
    
    boost::asio::async_write(socket_,
        boost::asio::buffer(data),
        [this, data](const boost::system::error_code& error, std::size_t length) {
            if (!error) {
                std::cout << "成功发送数据，大小: " << length << " 字节" << std::endl;
                doWrite(); // 继续发送下一个消息
            } else {
                std::cerr << "发送数据失败: " << error.message() << std::endl;
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
  std::cout << "准备发送数据到客户端 " << clientId << ":" << std::endl;
  std::cout << "- 原始数据大小: " << size << " 字节" << std::endl;
  
  // 使用小端序添加长度头
  packet.push_back(static_cast<uint8_t>(size & 0xFF));
  packet.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
  packet.push_back(static_cast<uint8_t>((size >> 16) & 0xFF));
  packet.push_back(static_cast<uint8_t>((size >> 24) & 0xFF));
  
  // 添加数据
  packet.insert(packet.end(), data.begin(), data.end());
  
  // 异步发送
  boost::asio::async_write(*clientSocket,
    boost::asio::buffer(packet),
    [this, clientId, size](const boost::system::error_code& error, std::size_t length) {
      if (error) {
        std::cerr << "发送数据到客户端 " << clientId << " 失败: " << error.message() << std::endl;
        removeClient(clientId);
      } else {
        std::cout << "成功发送数据到客户端 " << clientId << "，大小: " << length << " 字节" << std::endl;
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
    // 解析数据长度（小端序）
    uint32_t dataSize = static_cast<uint32_t>((*headerBuffer)[0]) |
                      (static_cast<uint32_t>((*headerBuffer)[1]) << 8) |
                      (static_cast<uint32_t>((*headerBuffer)[2]) << 16) |
                      (static_cast<uint32_t>((*headerBuffer)[3]) << 24);
    
    std::cout << "收到消息头部:" << std::endl;
    std::cout << "- 头部内容（十六进制）: ";
    for (const auto& byte : *headerBuffer) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
    std::cout << "- 解析出的数据长度: " << dataSize << " 字节" << std::endl;
    
    // 准备接收数据
    auto dataBuffer = std::make_shared<std::vector<uint8_t>>(dataSize);
    boost::asio::async_read(*socket,
        boost::asio::buffer(*dataBuffer),
        [this, socket, clientId, dataBuffer, headerBuffer](const boost::system::error_code& error, std::size_t length) {
            if (!error) {
                std::cout << "成功接收消息体:" << std::endl;
                std::cout << "- 实际接收大小: " << length << " 字节" << std::endl;
                std::cout << "- 消息内容（十六进制）: ";
                for (const auto& byte : *dataBuffer) {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                }
                std::cout << std::dec << std::endl;
                
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
                            std::cerr << "读取消息头部失败: " << error.message() << std::endl;
                            removeClient(clientId);
                        }
                    });
            } else {
                std::cerr << "读取消息体失败: " << error.message() << std::endl;
                removeClient(clientId);
            }
        });
}

} // namespace voicechat 