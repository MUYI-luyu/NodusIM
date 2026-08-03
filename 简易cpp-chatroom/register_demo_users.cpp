#include <iostream>
#include <string>
#include <hiredis/hiredis.h>
#include "user.h"
#include "report.h"
#include "client/register/readdata.h"
#include "client/register/hashpwd.h"

// 连接Redis
redisContext* connectRedis() {
    redisContext* redis = redisConnect("127.0.0.1", 6379);
    if (redis == NULL || redis->err) {
        if (redis) {
            std::cerr << "Redis连接错误: " << redis->errstr << std::endl;
            redisFree(redis);
        } else {
            std::cerr << "无法分配Redis上下文" << std::endl;
        }
        return nullptr;
    }
    std::cout << "Redis连接成功" << std::endl;
    return redis;
}

// 生成新的uid
std::string getNewUid(redisContext* redis) {
    redisReply* reply = (redisReply*)redisCommand(redis, "INCR newuid");
    if (!reply) {
        std::cerr << "Redis INCR newuid失败" << std::endl;
        return "0";
    }
    int uid = reply->integer;
    char buf[20];
    sprintf(buf, "%d", uid);
    freeReplyObject(reply);
    return buf;
}

// 注册用户
bool registerUser(redisContext* redis, const std::string& username, const std::string& password) {
    // 检查用户名是否已存在
    redisReply* reply = (redisReply*)redisCommand(redis, "GET username:%s", username.c_str());
    if (reply && reply->type != REDIS_REPLY_NIL) {
        std::cout << "用户名 " << username << " 已存在，跳过" << std::endl;
        freeReplyObject(reply);
        return false;
    }
    if (reply) freeReplyObject(reply);

    // 哈希密码
    PasswordHasher hasher;
    char pwd_copy[256];
    strncpy(pwd_copy, password.c_str(), sizeof(pwd_copy) - 1);
    pwd_copy[sizeof(pwd_copy) - 1] = '\0';
    hasher.hashPassword(pwd_copy);

    // 生成uid
    std::string uid = getNewUid(redis);

    // 创建用户对象
    user u;
    u.uid = uid;
    u.name = username;
    u.pwd = pwd_copy;  // 使用哈希后的密码
    u.email = "";
    u.stat = "offline";
    u.friendlist = {};
    u.grouplist = {};
    u.shieldlist = {};

    std::string userJson = u.toJson();

    // 写入Redis
    // 1. username -> uid
    reply = (redisReply*)redisCommand(redis, "SET username:%s %s", username.c_str(), uid.c_str());
    if (!reply) {
        std::cerr << "设置username失败" << std::endl;
        return false;
    }
    freeReplyObject(reply);

    // 2. user:uid -> json
    reply = (redisReply*)redisCommand(redis, "SET user:%s %s", uid.c_str(), userJson.c_str());
    if (!reply) {
        std::cerr << "设置user json失败" << std::endl;
        return false;
    }
    freeReplyObject(reply);

    // 3. 创建report
    report rpt;
    rpt.total_group_msg = 0;
    rpt.total_friend_msg = 0;
    std::string reportJson = rpt.toJson();

    reply = (redisReply*)redisCommand(redis, "SET report:%s %s", uid.c_str(), reportJson.c_str());
    if (!reply) {
        std::cerr << "设置report失败" << std::endl;
        return false;
    }
    freeReplyObject(reply);

    std::cout << "✓ 用户注册成功: " << username << " (uid: " << uid << ")" << std::endl;
    return true;
}

int main() {
    std::cout << "=== 聊天室Demo用户注册工具 ===" << std::endl;

    // 连接Redis
    redisContext* redis = connectRedis();
    if (!redis) {
        return 1;
    }

    // 注册测试用户
    std::cout << "\n开始注册测试用户..." << std::endl;

    registerUser(redis, "aaaa", "12345678");
    registerUser(redis, "bbbb", "12345678");
    registerUser(redis, "cccc", "12345678");

    std::cout << "\n注册完成！" << std::endl;
    std::cout << "可以使用以下账号登录：" << std::endl;
    std::cout << "  用户名: aaaa, 密码: 12345678" << std::endl;
    std::cout << "  用户名: bbbb, 密码: 12345678" << std::endl;
    std::cout << "  用户名: cccc, 密码: 12345678" << std::endl;

    // 清理
    redisFree(redis);
    return 0;
}
