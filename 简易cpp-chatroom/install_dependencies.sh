#!/bin/bash

set -e

echo "更新apt源..."
sudo apt update

echo "安装依赖开发包..."
sudo apt install -y \
    build-essential \
    libc6-dev \
    libhiredis-dev \
    libsodium-dev \
    libcurl4-openssl-dev \
    libgoogle-glog-dev

echo "依赖安装完成。"
