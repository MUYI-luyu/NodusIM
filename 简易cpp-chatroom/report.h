#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <map>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

struct report {
    std::set<std::string> friendapply;                 // 好友申请消息表
    std::map<std::string, int> chatfriend;             // 未读好友消息表
    std::unordered_map<std::string, int> chatgroup;    // 未读群聊消息表
    std::set<std::string> groupapply;                  // 未读群聊邀请表
    std::vector<std::string> notice;                   // 各种事务通知表

    int total_friend_msg = 0;                          // 未读好友消息总数
    int total_group_msg = 0;                           // 未读群聊消息总数

    // JSON 序列化
    std::string toJson() const;

    // JSON 反序列化
    static report fromJson(const std::string& s);
};
