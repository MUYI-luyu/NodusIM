#!/bin/bash
# 先编译
mkdir -p build
cd build || exit 1
cmake ..
make -j
cd ..

# 启动服务器
echo "Starting server..."

./build/server/server_app "$@"
