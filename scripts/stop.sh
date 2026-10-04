#!/bin/bash

# 停止所有微服务脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 服务配置（微服务名:基础端口:实例数）
SERVICES=(
    "traefik:8087:1"
    "user-service:8090:3"
    "friend-service:8100:3"
    "group-service:8110:3"
    "message-service:8120:3"
    "file-service:8130:3"
    "notification-service:8140:3"
)

# 获取服务的所有端口
GET_PORTS(){
  local service_info="$1"
  local service_name=$(echo "$service_info" | cut -d: -f1)
  local base_port=$(echo "$service_info" | cut -d: -f2)
  local instance_count=$(echo "$service_info" | cut -d: -f3)
  
  case "$service_name" in
    traefik) echo "$base_port";;
    *) 
      for ((i=0; i<instance_count; i++)); do
        echo $((base_port + i))
      done
      ;;
  esac
}

kill_by_pid(){
  local pid="$1"
  if [ -z "$pid" ]; then return 1; fi
  if ps -p "$pid" > /dev/null 2>&1; then
    kill "$pid" 2>/dev/null || true
    sleep 1
    if ps -p "$pid" > /dev/null 2>&1; then
      kill -9 "$pid" 2>/dev/null || true
      sleep 1
    fi
  fi
}

kill_by_port(){
  local port="$1"
  [ -z "$port" ] && return 1
  if command -v lsof >/dev/null 2>&1; then
    local pids
    pids=$(lsof -ti tcp:"$port" 2>/dev/null || true)
    if [ -n "$pids" ]; then
      kill $pids 2>/dev/null || true
      sleep 1
      pids=$(lsof -ti tcp:"$port" 2>/dev/null || true)
      if [ -n "$pids" ]; then
        kill -9 $pids 2>/dev/null || true
      fi
    fi
  elif command -v fuser >/dev/null 2>&1; then
    fuser -k ${port}/tcp 2>/dev/null || true
  fi
}

echo -e "${BLUE}🛑 停止IM系统微服务...${NC}"
echo -e "${YELLOW}ℹ️  注意: etcd 是系统服务，不会被此脚本停止${NC}"

# 停止各个服务
for service_info in "${SERVICES[@]}"; do
    service_name=$(echo "$service_info" | cut -d: -f1)
    base_port=$(echo "$service_info" | cut -d: -f2)
    instance_count=$(echo "$service_info" | cut -d: -f3)
    
    echo -e "${BLUE}🛑 停止 $service_name 服务...${NC}"
    
    # 停止所有实例
    for ((i=1; i<=instance_count; i++)); do
        port=$((base_port + i - 1))
        pid_file="scripts/pids/${service_name}-${i}.pid"
        
        if [ -f "$pid_file" ]; then
            PID=$(cat "$pid_file")
            if ps -p "$PID" > /dev/null 2>&1; then
                echo -e "${YELLOW}  停止 $service_name 实例 $i (端口: $port, PID: $PID)...${NC}"
                kill_by_pid "$PID"
                echo -e "${GREEN}  ✅ $service_name 实例 $i 已停止${NC}"
            else
                echo -e "${YELLOW}  ⚠️  $service_name 实例 $i 未运行或PID无效${NC}"
            fi
            rm -f "$pid_file"
        else
            echo -e "${YELLOW}  ⚠️  $service_name 实例 $i PID文件不存在${NC}"
        fi
        
        # 兜底：如果端口仍被占用，按端口清理
        if (command -v lsof >/dev/null 2>&1 && lsof -ti tcp:"$port" >/dev/null 2>&1) || \
           (command -v ss >/dev/null 2>&1 && ss -ltn | grep -q ":$port ") || \
           (command -v netstat >/dev/null 2>&1 && netstat -tuln 2>/dev/null | grep -q ":$port ")
        then
            echo -e "${YELLOW}  端口 $port 仍被占用，执行兜底清理...${NC}"
            kill_by_port "$port"
        fi
    done
    
    echo -e "${GREEN}✅ $service_name 服务所有实例已停止${NC}"
    echo ""
done

# 清理进程
echo -e "${BLUE}🧹 清理残留进程...${NC}"
pkill -f "traefik" 2>/dev/null || true
pkill -f "user-service" 2>/dev/null || true
pkill -f "friend-service" 2>/dev/null || true
pkill -f "group-service" 2>/dev/null || true
pkill -f "message-service" 2>/dev/null || true
pkill -f "file-service" 2>/dev/null || true
pkill -f "notification-service" 2>/dev/null || true

echo -e "${GREEN}🎉 所有微服务已停止！${NC}"
