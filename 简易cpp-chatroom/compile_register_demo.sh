#!/bin/bash

echo "编译Demo用户注册工具..."

g++ -o register_demo_users \
    register_demo_users.cpp \
    user.cpp \
    report.cpp \
    client/register/readdata.cpp \
    client/register/hashpwd.cpp \
    -I. \
    -I./client/register \
    -lhiredis \
    -lcrypto \
    -lsodium \
    -std=c++17

if [ $? -eq 0 ]; then
    echo "✓ 编译成功！"
    echo ""
    echo "运行以下命令注册测试用户："
    echo "  ./register_demo_users"
else
    echo "✗ 编译失败"
    exit 1
fi
