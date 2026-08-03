#!/bin/bash
# 先编译
mkdir -p build
cd build || exit 1
cmake ..
make -j
cd ..

# 启动客户端
echo "Starting client..."

./build/client/client_app "$@"
