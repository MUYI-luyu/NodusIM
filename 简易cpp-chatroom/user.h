#pragma once
#include <string>
#include <set>
#include "json.hpp"

using json = nlohmann::json;

class user {
public:
    std::string uid;
    std::string name;
    std::string email;
    std::string pwd;
    std::string stat;
    std::set<std::string> friendlist;
    std::set<std::string> grouplist;
    std::set<std::string> shieldlist;

    // 将当前对象转为 JSON 字符串
    std::string toJson() const;

    // 从 JSON 字符串构造对象
    static user fromJson(const std::string& s);
};
