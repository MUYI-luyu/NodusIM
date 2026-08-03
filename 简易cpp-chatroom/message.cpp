#include "message.h"

std::string message::toJson() const {
    json j;
    j["sender_uid"] = sender_uid;
    j["receiver_uid"] = receiver_uid;
    j["content"] = content;
    j["timestamp"] = timestamp;
    j["is_group"] = is_group;
    j["is_file"] = is_file;
    j["sender_name"] = sender_name;
    return j.dump();
}

message message::fromJson(const std::string& s) {
    message msg;
    try {
        json j = json::parse(s);
        msg.sender_uid = j.value("sender_uid", "");
        msg.receiver_uid = j.value("receiver_uid", "");
        msg.content = j.value("content", "");
        msg.timestamp = j.value("timestamp", "");
        msg.is_group = j.value("is_group", false);
        msg.is_file = j.value("is_file", false);
        msg.sender_name = j.value("sender_name", "");
    } catch (const json::parse_error& e) {
        std::cerr << "[JSON Parse Error] " << e.what() << "\nRaw Data: [" << s << "]\n";
    } catch (const json::type_error& e) {
        std::cerr << "[JSON Type Error] " << e.what() << "\nRaw Data: [" << s << "]\n";
    } catch (const std::exception& e) {
        std::cerr << "[JSON Other Error] " << e.what() << "\nRaw Data: [" << s << "]\n";
    }
    return msg;
}

std::string message::get_beijing_time() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t now_time = system_clock::to_time_t(now);
    now_time += 8 * 60 * 60;  // UTC +8

    std::tm* bj_time = std::gmtime(&now_time);
    std::ostringstream oss;
    oss << std::put_time(bj_time, "%Y-%m-%d %H:%M");
    return oss.str();
}

// messages 类实现

messages::messages(const std::vector<std::string>& msgs) : data(msgs) {}

std::string messages::toJson() const {
    json j;
    j["data"] = data;
    return j.dump();
}

messages messages::fromJson(const std::string& str) {
    messages m;
    try {
        json j = json::parse(str);
        m.data = j.at("data").get<std::vector<std::string>>();
    } catch (const json::parse_error& e) {
        std::cerr << "[messages::fromJson] JSON parse error: " << e.what() << "\nRaw: " << str << std::endl;
    } catch (const json::out_of_range& e) {
        std::cerr << "[messages::fromJson] Missing key: " << e.what() << "\nRaw: " << str << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[messages::fromJson] Error: " << e.what() << "\nRaw: " << str << std::endl;
    }
    return m;
}

// friendnamelist 类实现

friendnamelist::friendnamelist(const std::vector<std::string>& msgs) : data(msgs) {}

std::string friendnamelist::toJson() const {
    json j;
    j["data"] = data;
    return j.dump();
}

friendnamelist friendnamelist::fromJson(const std::string& str) {
    friendnamelist m;
    try {
        json j = json::parse(str);
        m.data = j.at("data").get<std::vector<std::string>>();
    } catch (const json::parse_error& e) {
        std::cerr << "[friendnamelist::fromJson] JSON parse error: " << e.what() << "\nRaw: " << str << std::endl;
    } catch (const json::out_of_range& e) {
        std::cerr << "[friendnamelist::fromJson] Missing key: " << e.what() << "\nRaw: " << str << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[friendnamelist::fromJson] Error: " << e.what() << "\nRaw: " << str << std::endl;
    }
    return m;
}

// file_block 类实现

std::string file_block::toJson() const {
    json j;
    j["sender_uid"] = sender_uid;
    j["receiver_uid"] = receiver_uid;
    j["fid"] = fid;
    j["filename"] = filename;
    j["timestamp"] = timestamp;
    j["offset"] = offset;
    j["is_group"] = is_group;
    j["is_file"] = is_file;
    return j.dump();
}

file_block file_block::fromJson(const std::string& s) {
    file_block msg;
    try {
        json j = json::parse(s);
        msg.sender_uid = j["sender_uid"];
        msg.receiver_uid = j["receiver_uid"];
        msg.fid = j["fid"];
        msg.filename = j["filename"];
        msg.timestamp = j["timestamp"];
        msg.offset = j["offset"];
        msg.is_group = j["is_group"];
        msg.is_file = j["is_file"];
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << "\nJSON content: " << s << std::endl;
        throw;
    }
    catch (const json::type_error& e) {
        std::cerr << "JSON type error: " << e.what() << "\nJSON content: " << s << std::endl;
        throw;
    }
    catch (const std::exception& e) {
        std::cerr << "JSON unknown error: " << e.what() << "\nJSON content: " << s << std::endl;
        throw;
    }
    return msg;
}

std::string file_block::get_beijing_time() {
        using namespace std::chrono;

        //获取当前系统时间（UTC）
        auto now = system_clock::now();
        std::time_t now_time = system_clock::to_time_t(now);

        //添加 8 小时偏移（UTC+8）
        now_time += 8 * 60 * 60;

        std::tm* bj_time = std::gmtime(&now_time);  //转为北京时间

        std::ostringstream oss;
        oss << std::put_time(bj_time, "%Y-%m-%d %H:%M");
        return oss.str();
}