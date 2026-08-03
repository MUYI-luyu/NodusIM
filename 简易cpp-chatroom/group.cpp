#include "group.h"

std::string group::toJson() const {
    json j;
    j["gid"] = gid;
    j["name"] = name;
    j["owner"] = owner;
    j["managelist"] = managelist;
    j["memberlist"] = memberlist;
    return j.dump();
}

group group::fromJson(const std::string& s) {
    json j = json::parse(s);
    group data;
    data.gid = j["gid"];
    data.name = j["name"];
    data.owner = j["owner"];
    data.managelist = j["managelist"].get<std::set<std::string>>();
    data.memberlist = j["memberlist"].get<std::set<std::string>>();
    return data;
}
