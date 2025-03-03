#!/bin/bash

# 确保在build/bin目录下
cd ../build/bin || exit 1

# 启动服务器（在后台运行）
./voice_server 8080 &
SERVER_PID=$!

# 等待服务器启动
sleep 2

# 启动第一个客户端（在后台运行）
./voice_client user1 127.0.0.1 8080 &
CLIENT1_PID=$!

# 启动第二个客户端（在后台运行）
./voice_client user2 127.0.0.1 8080 &
CLIENT2_PID=$!

# 等待用户输入来结束测试
echo "测试已启动。按回车键结束测试..."
read

# 结束所有进程
kill $SERVER_PID $CLIENT1_PID $CLIENT2_PID

echo "测试结束" 