#include "./userdata.h"

std::string userdata::newuser(std::string str){
    //拿到用户数据
    int i = 0;
    while(str[i] != ':') i++;
    std::string s = str.c_str() + i + 1;
    LOG_INFO("拿到json: " << s);
    user ud = user::fromJson(s);  
    //生成用户uid
    std::string nuid = newuid();
    ud.uid = nuid;
    //写入用户名->uid的键值对
    redisReply* reply = (redisReply*)redisCommand(redis, "SET username:%s %s", ud.name.c_str(), nuid.c_str());
    freeReplyObject(reply);
    //写入邮箱->uid的键值对
    reply = (redisReply*)redisCommand(redis, "SET email:%s %s", ud.email.c_str(), nuid.c_str());
    freeReplyObject(reply);
    //写入json字符串
    reply = (redisReply*)redisCommand(redis, "SET user:%s %s", nuid.c_str(), ud.toJson().c_str());
    freeReplyObject(reply);
    //将邮箱写入集合
    reply = (redisReply*)redisCommand(redis, "SADD email %s", ud.email.c_str());
    freeReplyObject(reply);
    //生成report数据
    report rpt;
    rpt.total_group_msg = rpt.total_friend_msg = 0;
    svreport(nuid, rpt.toJson());
    return nuid;
}

std::string userdata::newgroup(std::string &str){
    //拿到数据
    std::string gname,uid;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        gname.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid.push_back(str[t]);
    group gd;
    //生成gid
    std::string ngid = newgid();
    gd.name = gname; 
    gd.gid = ngid;
    gd.owner = uid;
    //写入组名->gid的键值对
    redisReply* reply = (redisReply*)redisCommand(redis, "SET groupname:%s %s", gd.name.c_str(), ngid.c_str());
    freeReplyObject(reply);
    //写入json字符串
    reply = (redisReply*)redisCommand(redis, "SET group:%s %s", ngid.c_str(), gd.toJson().c_str());
    freeReplyObject(reply);
    //获取用户结构体，修改数据
    user ud = GetUesr(uid);
    ud.grouplist.insert(ngid);
    setutoj(uid, ud.toJson());
    if(uid_to_socket.count(ud.uid)) sendMsg("user:"+ud.toJson(), uid_to_socket[ud.uid]);
    return ngid;
}


std::string userdata::newuid(){
    int i;
    redisReply* reply = NULL;
    reply = (redisReply*)redisCommand(redis, "INCR newuid");
    if (!reply) {
        LOG_ERROR("Redis INCR newuid failed");
        return "0";
    }
    i = reply->integer;
    char buf[20];
    sprintf(buf, "%d", i);
    freeReplyObject(reply);
    return buf;
}

std::string userdata::newgid(){
    int i;
    redisReply* reply = NULL;
    reply = (redisReply*)redisCommand(redis, "INCR newgid");
    if (!reply) {
        LOG_ERROR("Redis INCR newgid failed");
        return "0";
    }
    i = reply->integer;
    char buf[20];
    sprintf(buf, "%d", i);
    freeReplyObject(reply);
    return buf;
}

std::string userdata::newfid(){
    int i;
    redisReply* reply = NULL;
    reply = (redisReply*)redisCommand(redis, "INCR newfid");
    if (!reply) {
        LOG_ERROR("Redis INCR newfid failed");
        return "0";
    }
    i = reply->integer;
    char buf[20];
    sprintf(buf, "%d", i);
    freeReplyObject(reply);
    return buf;
}


bool userdata::RepeatEmail(const char * buf){
    int i = 0;
    while(buf[i] != ':') i++;
    std::string s = buf + i + 1;
    LOG_INFO("拿到email: " << s);
    redisReply* reply = (redisReply*)redisCommand(redis, "SISMEMBER email %s", s.c_str());
    if (!reply) {
        LOG_ERROR("Redis SISMEMBER email failed");
        return false;
    }
    bool ret = false;
    if(reply->integer == 1) ret = true;//存在
    freeReplyObject(reply);
    return ret;
}

//根据用户名返回用户id，若用户不存在则返回“norepeat”
std::string userdata::Getuid(const char * buf){
    LOG_INFO("拿到username: " << buf);
    redisReply* reply = (redisReply*)redisCommand(redis, "GET username:%s", buf);
    if (!reply) {
        LOG_ERROR("Redis GET username failed");
        return "norepeat";
    }
    if(reply->type == REDIS_REPLY_NIL){
        freeReplyObject(reply);
        return "norepeat";
    } else {
        std::string ret = reply->str;
        freeReplyObject(reply);
        return ret;
    }
}

//根据群聊名返回gid，若用户不存在则返回“norepeat”
std::string userdata::Getgid(const char * buf){
    redisReply* reply = (redisReply*)redisCommand(redis, "GET groupname:%s", buf);
    if (!reply) {
        LOG_ERROR("Redis GET groupname failed");
        return "norepeat";
    }
    if(reply->type == REDIS_REPLY_NIL){
        freeReplyObject(reply);
        return "norepeat";
    } else {
        std::string ret = reply->str;
        freeReplyObject(reply);
        return ret;
    }
}


std::string userdata::Getfid(std::string &uid, std::string &path, bool isgroupfile){
    redisReply* reply;
    if(isgroupfile == false)
        reply = (redisReply*)redisCommand(redis, "GET filehash:%s:%s", uid.c_str(), path.c_str());
    else reply = (redisReply*)redisCommand(redis, "GET gfilehash:%s:%s", uid.c_str(), path.c_str());
    if (!reply) {
        LOG_ERROR("Redis GET filehash failed");
        return "0";
    }
    if(reply->type == REDIS_REPLY_NIL){
        freeReplyObject(reply);
        std::string fid = newfid();
        if(isgroupfile == false)
            reply = (redisReply*)redisCommand(redis, "SET filehash:%s:%s %s", uid.c_str(), path.c_str(), fid.c_str());
        else
            reply = (redisReply*)redisCommand(redis, "SET gfilehash:%s:%s %s", uid.c_str(), path.c_str(), fid.c_str());
        freeReplyObject(reply);
        return fid;
    } else {
        std::string ret = reply->str;
        freeReplyObject(reply);
        return ret;
    }
}


std::string userdata::EmailGetuid(const char * buf){
    LOG_INFO("拿到email: " << buf);
    redisReply* reply = (redisReply*)redisCommand(redis, "GET email:%s", buf);
    if (!reply) {
        LOG_ERROR("Redis GET email failed");
        return "norepeat";
    }
    if(reply->type == REDIS_REPLY_NIL){
        freeReplyObject(reply);
        return "norepeat";
    } else {
        std::string ret = reply->str;
        freeReplyObject(reply);
        return ret;
    }
}


user userdata::GetUesr(std::string buf){
    redisReply* reply = (redisReply*)redisCommand(redis, "GET user:%s", buf.c_str());
    if (!reply) {
        std::cerr << "Redis command failed while loading user: " << buf << std::endl;
        user ret;
        ret.uid = buf;
        ret.name = "norepeat";
        ret.stat = "destroy";
        return ret;
    }

    if (reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        user ret;
        ret.uid = buf;
        ret.name = "norepeat";
        ret.stat = "destroy";
        return ret;
    }

    if (reply->type != REDIS_REPLY_STRING) {
        std::string errMsg = (reply->str ? reply->str : "(null)");
        freeReplyObject(reply);
        std::cerr << "Unexpected Redis reply type while loading user " << buf << ": " << errMsg << std::endl;
        user ret;
        ret.uid = buf;
        ret.name = "norepeat";
        ret.stat = "destroy";
        return ret;
    }

    try {
        user ret = user::fromJson(reply->str);
        freeReplyObject(reply);
        return ret;
    } catch (const std::exception& e) {
        std::string badJson = reply->str ? reply->str : "(null)";
        freeReplyObject(reply);
        std::cerr << "Invalid JSON from Redis for user " << buf << ": " << e.what()
                  << ", content: " << badJson << std::endl;
        user ret;
        ret.uid = buf;
        ret.name = "norepeat";
        ret.stat = "destroy";
        return ret;
    }
}

std::string userdata::GetGroup(std::string buf){
    redisReply* reply = (redisReply*)redisCommand(redis, "GET group:%s", buf.c_str());
    std::string ret = "norepeat";
    if(reply == NULL || reply->type == REDIS_REPLY_NIL || reply->type != REDIS_REPLY_STRING)
        return ret;
    ret = reply->str;
    freeReplyObject(reply);
    return ret;
}

bool userdata::DELUesr(std::string uid){
    redisReply* reply = (redisReply*)redisCommand(redis, "DEL user:%s", uid.c_str());
    if (!reply) {
        return false;
    }
    bool ret = false;
    if (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0)
        ret = true;
    freeReplyObject(reply);
    return ret;
}

bool userdata::setutoj(std::string nuid, std::string s){
    redisReply*reply = (redisReply*)redisCommand(redis, "SET user:%s %s", nuid.c_str(), s.c_str());
    if (!reply) {
        return false;
    }
    freeReplyObject(reply);
    return true;
}
bool userdata::setgtoj(std::string ngid, std::string s){
    redisReply*reply = (redisReply*)redisCommand(redis, "SET group:%s %s", ngid.c_str(), s.c_str());
    if (!reply) {
        return false;
    }
    freeReplyObject(reply);
    return true;
}


std::string userdata::AddFrd(std::string name1, std::string uid2){
    std::string js = u_report(uid2);
    report rpt;
    if(js != "none") 
        rpt = report::fromJson(js);
    rpt.friendapply.insert(name1);
    js = rpt.toJson();
    svreport(uid2, js);
    return js;
}

bool userdata::jguid(std::string uid){
    redisReply* reply = (redisReply*)redisCommand(redis, "EXISTS user:%s", uid.c_str());
    bool ret = false;
    if (reply && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1)
        ret = true;
    if (reply) freeReplyObject(reply);
    return ret;
}

std::string userdata::u_report(std::string uid){
    redisReply* reply = (redisReply*)redisCommand(redis, "GET report:%s",uid.c_str());
    LOG_WARN("GET uid: " << uid);

    if(reply == NULL || reply->type == REDIS_REPLY_NIL || reply->type != REDIS_REPLY_STRING){
        if(reply) {
            LOG_WARN("return none, reply->str: " << (reply->str ? reply->str : "(null)"));
            freeReplyObject(reply);
        }
        return "none";
    }
    std::string ret = reply->str;
    freeReplyObject(reply);
    return ret;
}

bool userdata::svreport(std::string uid, std::string js){
    //写入json字符串
    redisReply* reply = (redisReply*)redisCommand(redis, "SET report:%s %s", uid.c_str(), js.c_str());
    if (!reply) {
        return false;
    }
    freeReplyObject(reply);
    return true;
}
int userdata::llen(std::string uid1, std::string uid2){
    // 先获取list长度
    std::string chat_key = "chat:" + uid1 + ":" + uid2;
    redisReply* reply_len = (redisReply*)redisCommand(redis, "LLEN %s", chat_key.c_str());
    if (!reply_len || reply_len->type != REDIS_REPLY_INTEGER) {
        if(reply_len) freeReplyObject(reply_len);
        return 0; // 失败返回空
    }
    int len = (int)reply_len->integer;
    freeReplyObject(reply_len);
    return len;
}

int userdata::gllen(std::string gid){
    // 先获取list长度
    std::string chat_key = "gchat:" + gid;
    redisReply* reply_len = (redisReply*)redisCommand(redis, "LLEN %s", chat_key.c_str());
    if (!reply_len || reply_len->type != REDIS_REPLY_INTEGER) {
        if(reply_len) freeReplyObject(reply_len);
        return 0; // 失败返回空
    }
    int len = (int)reply_len->integer;
    freeReplyObject(reply_len);
    return len;
}



std::vector<std::string> userdata::lrange(std::string uid1, std::string uid2, int start, int stop){
    std::vector<std::string> result;
    std::string chat_key = "chat:" + uid1 + ":" + uid2;
    int len = 0;
    if (len == llen(uid1, uid2)) return result; // 无消息直接返回

    redisReply* reply = (redisReply*)redisCommand(redis, "LRANGE %s %d %d", chat_key.c_str(), start, stop);
    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        if(reply) freeReplyObject(reply);
        return result;
    }

    for (int i = 0; i < reply->elements; i++) {
        if(reply->element[i]->type == REDIS_REPLY_STRING)
            result.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);

    return result;
}

std::vector<std::string> userdata::glrange(std::string gid, int start, int stop){
    std::vector<std::string> result;
    std::string chat_key = "gchat:" + gid;
    int len = 0;
    if (len == gllen(gid)) return result; // 无消息直接返回

    redisReply* reply = (redisReply*)redisCommand(redis, "LRANGE %s %d %d", chat_key.c_str(), start, stop);
    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        if(reply) freeReplyObject(reply);
        return result;
    }

    for (int i = 0; i < reply->elements; i++) {
        if(reply->element[i]->type == REDIS_REPLY_STRING)
            result.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);

    return result;
}

void userdata::savechat(std::string js){
    message sendm = message::fromJson(js);
    std::string chat_key = "chat:" + sendm.receiver_uid + ":" + sendm.sender_uid;
    redisReply* reply = (redisReply*)redisCommand(redis, "LPUSH %s %s", chat_key.c_str(), js.c_str());
    freeReplyObject(reply);
}


void userdata::savechat2( std::string js){
    message sendm = message::fromJson(js);
    std::string chat_key = "chat:" + sendm.sender_uid + ":" + sendm.receiver_uid;
    redisReply* reply = (redisReply*)redisCommand(redis, "LPUSH %s %s", chat_key.c_str(), js.c_str());
    freeReplyObject(reply);
}

void userdata::savegchat(std::string js){
    message sendm = message::fromJson(js);
    std::string chat_key = "gchat:" + sendm.receiver_uid;
    redisReply* reply = (redisReply*)redisCommand(redis, "LPUSH %s %s", chat_key.c_str(), js.c_str());
    freeReplyObject(reply);
}

void userdata::disbandgroup(std::string gid, std::string gname){
    std::string chat_key = "gchat:" + gid;
    redisReply* reply = (redisReply*)redisCommand(redis, "DEL %s", chat_key.c_str());
    freeReplyObject(reply);
    reply = (redisReply*)redisCommand(redis, "DEL groupname:%s", gname.c_str());
    freeReplyObject(reply);
}

void userdata::destroy_user(std::string uid, std::string uname, std::string email){
    //username对于uid的映射清除
    redisReply* reply = (redisReply*)redisCommand(redis, "DEL username:%s", uname.c_str());
    freeReplyObject(reply);
    //清除该用户id的report
    reply = (redisReply*)redisCommand(redis, "DEL report:%s", uid.c_str());
    freeReplyObject(reply);
    //清除关于该用户的所有聊天
    //清除email表里该用户的email和email:%s的映射
    reply = (redisReply*)redisCommand(redis, "DEL email:%s", email.c_str());
    freeReplyObject(reply);
    reply = (redisReply*)redisCommand(redis, "SREM email %s", email.c_str());
    freeReplyObject(reply);
}
