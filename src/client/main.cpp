#include "voice_client.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <limits>
#include <iomanip>

using namespace voicechat;

static bool running = true;

void signalHandler(int) {
    running = false;
}

void printHelp(VoiceClient& client) {
    std::cout << "可用命令：" << std::endl;
    std::cout << "  join <房间ID> - 加入语音房间" << std::endl;
    std::cout << "  leave - 离开当前房间" << std::endl;
    std::cout << "  mute - 静音" << std::endl;
    std::cout << "  unmute - 取消静音" << std::endl;
    std::cout << "  quit - 退出程序" << std::endl;
    std::cout << "  help - 显示此帮助信息" << std::endl;
    std::cout << std::endl;

    // 获取并显示可用频道列表
    auto rooms = client.getAvailableRooms();
    if (!rooms.empty()) {
        std::cout << "当前可用频道：" << std::endl;
        std::cout << std::setw(20) << std::left << "频道ID" << "在线人数" << std::endl;
        std::cout << std::string(40, '-') << std::endl;
        
        for (const auto& [roomId, count] : rooms) {
            std::cout << std::setw(20) << std::left << roomId << count << std::endl;
        }
    } else {
        std::cout << "当前没有可用的频道" << std::endl;
    }
    std::cout << std::endl;
}

// 处理用户命令
void processCommand(VoiceClient& client, const std::string& line) {
    if (line.empty()) {
        return;
    }

    std::istringstream iss(line);
    std::string command;
    iss >> command;

    if (command == "help") {
        printHelp(client);
    }
    else if (command == "join") {
        std::string roomId;
        std::getline(iss >> std::ws, roomId);  // 读取剩余部分作为房间ID，去除前导空格
        if (roomId.empty()) {
            std::cout << "请指定要加入的频道ID。可用的频道：" << std::endl;
            auto rooms = client.getAvailableRooms();
            for (const auto& [id, count] : rooms) {
                std::cout << "  " << id << " (" << count << " 人在线)" << std::endl;
            }
            return;
        }
        if (client.joinRoom(roomId)) {
            std::cout << "已加入房间: " << roomId << std::endl;
        } else {
            std::cout << "加入房间失败" << std::endl;
        }
    }
    else if (command == "leave") {
        if (client.leaveRoom()) {
            std::cout << "已离开房间" << std::endl;
        } else {
            std::cout << "离开房间失败" << std::endl;
        }
    }
    else if (command == "mute") {
        client.setMuted(true);
        std::cout << "已静音" << std::endl;
    }
    else if (command == "unmute") {
        client.setMuted(false);
        std::cout << "已取消静音" << std::endl;
    }
    else if (command == "quit") {
        throw std::runtime_error("quit");  // 使用异常来退出主循环
    }
    else {
        std::cout << "未知命令。输入 'help' 查看可用命令。" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "用法: " << argv[0] << " <用户ID> <服务器地址> <端口>" << std::endl;
        return 1;
    }

    // 注册信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        std::string userId = argv[1];
        std::string host = argv[2];
        uint16_t port = static_cast<uint16_t>(std::stoi(argv[3]));

        // 创建客户端实例
        VoiceClient client(userId);

        // 连接到服务器
        if (!client.connect(host, port)) {
            std::cerr << "无法连接到服务器" << std::endl;
            return 1;
        }

        std::cout << "已连接到服务器。输入 'help' 查看可用命令。" << std::endl;

        // 主循环
        std::string line;
        while (running) {
            std::cout << "> ";  // 命令提示符
            std::cout.flush();  // 确保提示符立即显示
            
            // 读取一行输入
            if (!std::getline(std::cin, line)) {
                break;  // 处理EOF或输入错误
            }

            try {
                processCommand(client, line);
            } catch (const std::runtime_error& e) {
                if (std::string(e.what()) == "quit") {
                    break;  // 正常退出
                }
                throw;  // 重新抛出其他异常
            }
        }

        // 断开连接
        client.disconnect();
        std::cout << "已断开连接" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 