#include "Redis.h"
#include <iostream>

// 连接数据库
redisContext* connectRedis() {
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (c == nullptr || c->err) {
        std::cerr << "连接 Redis 出错: " << (c ? c->errstr : "无法分配内存") << std::endl;
        return nullptr;
    }
    return c;
}
