#include "voice_client.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <limits>
#include <iomanip>
#include <csignal>
#include <atomic>

using namespace voicechat;

std::atomic<bool> g_running(true);
std::shared_ptr<VoiceClient> g_client;

void signalHandler(int signum) {
    if (signum == SIGINT) {
        std::cout << "\n正在退出..." << std::endl;
        g_running = false;
    }
}

void quit() {
    std::cout << "正在退出客户端..." << std::endl;
    g_running = false;
    if (g_client) {
        try {
            // 先离开当前房间
            g_client->leaveRoom();
            // 等待一小段时间确保消息发送
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // 断开连接
            g_client->disconnect();
            // 清理资源
            g_client.reset();
        } catch (const std::exception& e) {
            std::cerr << "退出时发生错误: " << e.what() << std::endl;
        }
    }
    // 不直接调用 exit，让程序自然退出以确保资源正确清理
}

void printHelp() {
    std::cout << "可用命令：" << std::endl;
    std::cout << "  join <房间ID> - 加入语音房间" << std::endl;
    std::cout << "  leave - 离开当前房间" << std::endl;
    std::cout << "  mute - 静音" << std::endl;
    std::cout << "  unmute - 取消静音" << std::endl;
    std::cout << "  quit - 退出程序" << std::endl;
    std::cout << "  help - 显示此帮助信息" << std::endl;
    std::cout << std::endl;

    // 获取并显示可用频道列表
    auto rooms = g_client->getAvailableRooms();
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
void processCommand(VoiceClient* client, const std::string& line) {
    if (line.empty()) {
        return;
    }

    std::istringstream iss(line);
    std::string command;
    iss >> command;
    if (command == "help") {
        printHelp();
    }
    else if (command == "join") {
        std::string roomId;
        std::getline(iss >> std::ws, roomId);  // 读取剩余部分作为房间ID，去除前导空格
        if (roomId.empty()) {
            std::cout << "请指定要加入的频道ID。可用的频道:" << std::endl;
            auto rooms = client->getAvailableRooms();
            for (const auto& [id, count] : rooms) {
                std::cout << "  " << id << " (" << count << " 人在线)" << std::endl;
            }
            return;
        }
        if (client->joinRoom(roomId)) {
            std::cout << "已加入房间: " << roomId << std::endl;
        } else {
            std::cout << "加入房间失败" << std::endl;
        }
    }
    else if (command == "leave") {
        if (client->leaveRoom()) {
            std::cout << "已离开房间" << std::endl;
        } else {
            std::cout << "离开房间失败" << std::endl;
        }
    }
    else if (command == "mute") {
        client->setMuted(true);
        std::cout << "已静音" << std::endl;
    }
    else if (command == "unmute") {
        client->setMuted(false);
        std::cout << "已取消静音" << std::endl;
    }
    else if (command == "quit" || command == "exit") {
        quit();
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

    // 设置信号处理
    std::signal(SIGINT, signalHandler);

    try {
        std::string userId = argv[1];
        std::string host = argv[2];
        uint16_t port = static_cast<uint16_t>(std::stoi(argv[3]));

        g_client = std::make_shared<VoiceClient>(userId);
        
        if (!g_client->connect(host, port)) {
            std::cerr << "连接服务器失败" << std::endl;
            return 1;
        }

        std::cout << "已连接到服务器" << std::endl;
        printHelp();

        std::string command;
        while (g_running) {
            std::cout << "> ";
            std::cout.flush();
            
            if (!std::getline(std::cin, command) || !g_running) {
                break;
            }

            if (command == "quit" || command == "exit") {
                quit();
                break;
            }

            processCommand(g_client.get(), command);
        }

        // 如果是因为Ctrl+C退出的循环
        quit();
    } catch (const std::exception& e) {
        std::cerr << "发生错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 