#!/bin/bash

# 定义颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 定义变量
SERVER_PORT=8080
BUILD_DIR="../build"
BIN_DIR="$BUILD_DIR/bin"

# 确保在正确的目录
cd "$(dirname "$0")" || exit 1

# 检查可执行文件是否存在
if [ ! -f "$BIN_DIR/voice_server" ] || [ ! -f "$BIN_DIR/voice_client" ]; then
    echo -e "${RED}错误：找不到服务器或客户端可执行文件。${NC}"
    echo "请先编译项目。"
    exit 1
fi

# 清理旧进程
echo -e "${BLUE}清理可能存在的旧进程...${NC}"
pkill -f "voice_server"
pkill -f "voice_client"
sleep 1

# 启动服务器
echo -e "${GREEN}启动语音聊天服务器...${NC}"
$BIN_DIR/voice_server $SERVER_PORT &
SERVER_PID=$!

# 等待服务器启动
echo "等待服务器启动..."
sleep 2

# 启动第一个客户端
echo -e "${GREEN}启动第一个客户端 (user1)...${NC}"
$BIN_DIR/voice_client user1 127.0.0.1 $SERVER_PORT &
CLIENT1_PID=$!

# 等待第一个客户端连接
sleep 1

# 启动第二个客户端
echo -e "${GREEN}启动第二个客户端 (user2)...${NC}"
$BIN_DIR/voice_client user2 127.0.0.1 $SERVER_PORT &
CLIENT2_PID=$!

# 打印测试说明
echo
echo -e "${BLUE}测试说明：${NC}"
echo "1. 两个客户端已自动加入主频道"
echo "2. 可以在各自的客户端窗口中使用以下命令："
echo "   - mute    : 静音"
echo "   - unmute  : 取消静音"
echo "   - quit    : 退出程序"
echo
echo -e "${RED}按回车键结束测试...${NC}"
read

# 清理进程
echo -e "${BLUE}清理进程...${NC}"
kill $SERVER_PID $CLIENT1_PID $CLIENT2_PID 2>/dev/null
pkill -f "voice_server"
pkill -f "voice_client"

echo -e "${GREEN}测试结束${NC}" 