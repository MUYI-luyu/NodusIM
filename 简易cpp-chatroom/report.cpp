#include "report.h"
#include <iostream>

std::string report::toJson() const {
    json j;
    j["friendapply"] = friendapply;
    j["chatfriend"] = chatfriend;
    j["chatgroup"] = chatgroup;
    j["groupapply"] = groupapply;
    j["notice"] = notice;
    j["total_friend_msg"] = total_friend_msg;
    j["total_group_msg"] = total_group_msg;
    return j.dump();
}

report report::fromJson(const std::string& s) {
    report data;
    if(s.size() != 0 && s[0] != '{') {
        std::cerr << "error rpt js: " << s << std::endl;
        return data;
    }
    try {
        json j = json::parse(s);

        data.friendapply = j["friendapply"].get<std::set<std::string>>();
        data.chatfriend = j["chatfriend"].get<std::map<std::string, int>>();
        data.chatgroup = j["chatgroup"].get<std::unordered_map<std::string, int>>();
        data.groupapply = j["groupapply"].get<std::set<std::string>>();
        data.notice = j["notice"].get<std::vector<std::string>>();
        data.total_friend_msg = j["total_friend_msg"].get<int>();
        data.total_group_msg = j["total_group_msg"].get<int>();

    } catch (const json::parse_error& e) {
        std::cerr << "[fromJson] JSON parse error: " << e.what()
                  << "\nRaw: " << s << std::endl;
    }
    return data;
}
