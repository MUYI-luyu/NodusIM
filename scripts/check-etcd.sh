#!/bin/bash

# 检查 etcd 状态脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}🔍 检查 etcd 状态...${NC}"

# 检查 etcd 进程
if pgrep -x "etcd" > /dev/null; then
    echo -e "${GREEN}✅ etcd 进程正在运行${NC}"
    
    # 检查 etcd 端口
    if ss -tuln 2>/dev/null | grep -q ":2379 "; then
        echo -e "${GREEN}✅ etcd 端口 2379 已开放${NC}"
        
        # 尝试连接 etcd
        if command -v etcdctl >/dev/null 2>&1; then
            if etcdctl endpoint health --endpoints=http://127.0.0.1:2379 >/dev/null 2>&1; then
                echo -e "${GREEN}✅ etcd 服务健康检查通过${NC}"
                echo -e "${GREEN}🎉 etcd 系统服务运行正常，可以启动 IM 服务${NC}"
                exit 0
            else
                echo -e "${YELLOW}⚠️  etcd 端口开放但健康检查失败${NC}"
            fi
        else
            echo -e "${YELLOW}⚠️  etcdctl 未安装，无法进行健康检查${NC}"
            echo -e "${GREEN}✅ 但 etcd 进程和端口都正常${NC}"
        fi
    else
        echo -e "${RED}❌ etcd 端口 2379 未开放${NC}"
        exit 1
    fi
else
    echo -e "${RED}❌ etcd 进程未运行${NC}"
    echo -e "${YELLOW}💡 请先启动 etcd 系统服务:${NC}"
    echo -e "   ${YELLOW}sudo systemctl start etcd${NC}"
    echo -e "   或"
    echo -e "   ${YELLOW}sudo systemctl enable etcd && sudo systemctl start etcd${NC}"
    exit 1
fi