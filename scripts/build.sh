#!/bin/bash

# 构建所有微服务脚本

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 变量定义
BUILD_DIR="bin"
CMD_DIR="cmd"
GO="go"

echo -e "${BLUE}🔨 构建所有微服务...${NC}"

# 创建构建目录
mkdir -p "$BUILD_DIR"

# 构建计数器
BUILT_COUNT=0
TOTAL_COUNT=6

echo -e "${BLUE}总共需要构建 $TOTAL_COUNT 个服务${NC}"

# 构建函数
build_service() {
    local service_name="$1"
    local main_file="$CMD_DIR/$service_name/main.go"
    local output_file="$BUILD_DIR/$service_name"
    
    echo -e "${YELLOW}构建 $service_name...${NC}"
    
    # 检查主文件是否存在
    if [ ! -f "$main_file" ]; then
        echo -e "${RED}❌ 主文件不存在: $main_file${NC}"
        return 1
    fi
    
    # 构建服务
    if $GO build -o "$output_file" "$main_file" 2>/dev/null; then
        echo -e "${GREEN}✅ $service_name 构建成功${NC}"
        return 0
    else
        echo -e "${RED}❌ $service_name 构建失败${NC}"
        return 1
    fi
}

# 构建所有服务
build_service "user-service" && ((BUILT_COUNT++))
build_service "friend-service" && ((BUILT_COUNT++))
build_service "group-service" && ((BUILT_COUNT++))
build_service "message-service" && ((BUILT_COUNT++))
build_service "file-service" && ((BUILT_COUNT++))
build_service "notification-service" && ((BUILT_COUNT++))

# 输出构建结果
echo ""
if [ $BUILT_COUNT -eq $TOTAL_COUNT ]; then
    echo -e "${GREEN}🎉 所有微服务构建完成 ($BUILT_COUNT/$TOTAL_COUNT)${NC}"
    exit 0
else
    echo -e "${YELLOW}⚠️  部分微服务构建失败 ($BUILT_COUNT/$TOTAL_COUNT)${NC}"
    exit 1
fi