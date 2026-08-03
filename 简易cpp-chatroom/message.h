#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "json.hpp"

using json = nlohmann::json;

struct message {
    std::string sender_uid;      // 发送者 UID
    std::string receiver_uid;    // 接收者 UID 或群 ID
    std::string content;         // 消息内容
    std::string timestamp;       // 时间戳（可使用ISO 8601格式或其他字符串）
    bool is_group = false;       // 是否为群聊消息
    bool is_file = false;        // 是否为文件消息（可扩展用）
    std::string sender_name;      // 发送者 UID
    
    std::string toJson() const;  // 转为 JSON 字符串
    static message fromJson(const std::string& s); // 从 JSON 字符串构造 message 对象
    static std::string get_beijing_time();  // 获取北京时间
};

class messages {
public:
    std::vector<std::string> data;

    messages() = default;
    messages(const std::vector<std::string>& msgs);
    
    std::string toJson() const;  // 序列化为 JSON 字符串
    static messages fromJson(const std::string& str);  // 从 JSON 字符串反序列化为 messages 对象
};

class friendnamelist {
public:
    std::vector<std::string> data;

    friendnamelist() = default;
    friendnamelist(const std::vector<std::string>& msgs);

    std::string toJson() const;  // 序列化为 JSON 字符串
    static friendnamelist fromJson(const std::string& str);  // 从 JSON 字符串反序列化为 friendnamelist 对象
};

struct file_block {
    std::string sender_uid;      // 发送者 UID
    std::string receiver_uid;    // 接收者 UID 或群 ID
    std::string fid;             // 文件id
    std::string filename;        // 文件名 
    std::string timestamp;       // 时间戳（可使用ISO 8601格式或其他字符串）
    long long offset;
    bool is_group = false;       // 是否为群聊消息
    bool is_file = false;        // 是否为文件消息（可扩展用）

    std::string toJson() const;  // 转为 JSON 字符串
    static file_block fromJson(const std::string& s);  // 从 JSON 字符串构造 file_block 对象
    static std::string get_beijing_time();  // 获取北京时间
};
