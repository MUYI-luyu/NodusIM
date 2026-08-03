#ifndef REDIS_HPP
#define REDIS_HPP

#include <hiredis/hiredis.h>

// 声明 connectRedis 函数
redisContext* connectRedis();

#endif // REDIS_HPP
