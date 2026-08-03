#pragma once
#include <string>
#include <set>
#include "json.hpp"

using json = nlohmann::json;

struct group {
    std::string gid;  // 群组ID
    std::string name; // 群组名称
    std::string owner; // 群主
    std::set<std::string> managelist; // 管理员列表
    std::set<std::string> memberlist; // 成员列表

    // 将当前对象转为 JSON 字符串
    std::string toJson() const;

    // 从 JSON 字符串构造对象
    static group fromJson(const std::string& s);
};
