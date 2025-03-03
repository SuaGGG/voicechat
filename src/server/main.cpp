#include "voice_server.hpp"
#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace voicechat;

static bool running = true;
static VoiceServer* serverPtr = nullptr;

void signalHandler(int) {
  running = false;
  if (serverPtr) {
    serverPtr->stop();
  }
}

void printServerStats(const VoiceServer& server) {
  //std::cout << "\033[2J\033[H";  // 清屏并移动光标到开始位置
  std::cout << "=== Voice Chat Server Statistics ===" << std::endl;
  std::cout << "Connected clients: " << server.getConnectedClientsCount() << std::endl;
  
  std::cout << "\nActive Rooms:" << std::endl;
  std::cout << std::setw(20) << "Room ID" << std::setw(15) << "Participants" << std::endl;
  std::cout << std::string(35, '-') << std::endl;
  
  auto roomStats = server.getRoomParticipantCounts();
  for (const auto& [roomId, count] : roomStats) {
    std::cout << std::setw(20) << roomId << std::setw(15) << count << std::endl;
  }
  
  std::cout << "\nPress Ctrl+C to stop the server" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    return 1;
  }

  try {
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));
    
    // 创建服务器实例
    VoiceServer server(port);
    serverPtr = &server;

    // 注册信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // 启动服务器
    if (!server.start()) {
      std::cerr << "Failed to start server" << std::endl;
      return 1;
    }
    std::cout << "Server is running on port " << port << std::endl;

    printServerStats(server);
    // 主循环
    while (running) {
      // printServerStats(server);
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\nShutting down server..." << std::endl;
    server.stop();
    serverPtr = nullptr;

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
} 