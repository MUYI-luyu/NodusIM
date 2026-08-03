#include "user.h"
#include <iostream>

std::string user::toJson() const {
    json j;
    j["uid"] = uid;
    j["name"] = name;
    j["email"] = email;
    j["pwd"] = pwd;
    j["stat"] = stat;
    j["friendlist"] = friendlist;
    j["grouplist"] = grouplist;
    j["shieldlist"] = shieldlist;
    return j.dump();
}

user user::fromJson(const std::string& s) {
    user data;
    try {
        json j = json::parse(s);
        data.uid = j["uid"];
        data.name = j["name"];
        data.email = j["email"];
        data.pwd = j["pwd"];
        data.stat = j["stat"];
        data.friendlist = j["friendlist"].get<std::set<std::string>>();
        data.grouplist = j["grouplist"].get<std::set<std::string>>();
        data.shieldlist = j["shieldlist"].get<std::set<std::string>>();
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
    return data;
}
