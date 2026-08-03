#include "./handle.h"

std::unordered_map<int , time_t> uslast_active;
std::mutex map_mutex;

//处理用户传来的请求
int handler::handle(void){
    if (str.size() < 4) {
        return 0;
    }
    if(str[0] == 'j' && str[1] == 'r' && str[2] == 'n' && str[3] == 'm') jrnm();
    else if(str[0] == 'r' && str[1] == 'g' && str[2] == 's' && str[3] == 't') torgst();
    else if(str[0] == 'j' && str[1] == 'r' && str[2] == 'u' && str[3] == 'd') jrud();
    else if(str[0] == 'j' && str[1] == 'r' && str[2] == 'e' && str[3] == 'm') jrem();
    else if(str[0] == 'j' && str[1] == 'r' && str[2] == 'n' && str[3] == 'e') jrne();
    else if(str[0] == 'j' && str[1] == 'r' && str[2] == 'g' && str[3] == 'n') jrgn();
    else if(str[0] == 'j' && str[1] == 'r' && str[2] == 'f' && str[3] == 'd') jrfd();
    else if(str[0] == 'p' && str[1] == 'w' && str[2] == 'l' && str[3] == 'g') pwlg();
    else if(str[0] == 'f' && str[1] == 'd' && str[2] == 'p' && str[3] == 'd') fdpd();
    else if(str[0] == 'e' && str[1] == 'm' && str[2] == 'l' && str[3] == 'g') emlg();
    else if(str[0] == 'u' && str[1] == 'n' && str[2] == 'l' && str[3] == 'g') unlg(str);
    else if(str[0] == 'u' && str[1] == 'u' && str[2] == 'l' && str[3] == 'g') uulg(str);
    else if(str[0] == 'a' && str[1] == 'd' && str[2] == 'f' && str[3] == 'r') adfr();
    else if(str[0] == 'a' && str[1] == 'd' && str[2] == 'g' && str[3] == 'p') adgp();
    else if(str[0] == 'a' && str[1] == 'd' && str[2] == 'm' && str[3] == 'n') admn();
    else if(str[0] == 'd' && str[1] == 'l' && str[2] == 'm' && str[3] == 'n') dlmn();
    else if(str[0] == 'a' && str[1] == 'd' && str[2] == 'f' && str[3] == '(') adfok();
    else if(str[0] == 'a' && str[1] == 'd' && str[2] == 'g' && str[3] == '(') adgok();
    else if(str[0] == 'g' && str[1] == 't' && str[2] == 'r' && str[3] == 'p') gtrp();
    else if(str[0] == 'g' && str[1] == 't' && str[2] == 'u' && str[3] == 's') gtus();
    else if(str[0] == 'g' && str[1] == 't' && str[2] == 'g' && str[3] == 'p') gtgp();
    else if(str[0] == 'g' && str[1] == 't' && str[2] == 'u' && str[3] == 'd') gtud();
    else if(str[0] == 'g' && str[1] == 't' && str[2] == 'f' && str[3] == 'i') gtfi();
    else if(str[0] == 'g' && str[1] == 't' && str[2] == 'n' && str[3] == 'm') gtnm();
    else if(str[0] == 'g' && str[1] == 'g' && str[2] == 'n' && str[3] == 'm') ggnm();
    else if(str[0] == 'r' && str[1] == 'd' && str[2] == 'n' && str[3] == 't') rdnt();
    else if(str[0] == 'r' && str[1] == 'm' && str[2] == 'n' && str[3] == 't') rmnt();
    else if(str[0] == 'r' && str[1] == 'm' && str[2] == 'f' && str[3] == 'd') rmfd();
    else if(str[0] == 'r' && str[1] == 'm' && str[2] == 'g' && str[3] == 'p') rmgp(str);
    else if(str[0] == 'c' && str[1] == 't' && str[2] == 'm' && str[3] == 's') ctms();
    else if(str[0] == 'g' && str[1] == 'c' && str[2] == 't' && str[3] == 'm') gctm();
    else if(str[0] == 'c' && str[1] == 't' && str[2] == 'g' && str[3] == 'p') ctgp();
    else if(str[0] == 's' && str[1] == 'd' && str[2] == 'm' && str[3] == 's') sdms();
    else if(str[0] == 's' && str[1] == 'd' && str[2] == 'g' && str[3] == 'm') sdgm();
    else if(str[0] == 'r' && str[1] == 'd' && str[2] == 'p' && str[3] == 'g') rdpg();
    else if(str[0] == 'r' && str[1] == 'd' && str[2] == 'g' && str[3] == 'c') rdgc();
    else if(str[0] == 's' && str[1] == 'v' && str[2] == 'r' && str[3] == 'p') svrp();
    else if(str[0] == 'n' && str[1] == 'd' && str[2] == 'm' && str[3] == 's') ndms();
    else if(str[0] == 'n' && str[1] == 'd' && str[2] == 'g' && str[3] == 'm') ndgm();
    else if(str[0] == 's' && str[1] == 'h' && str[2] == 'f' && str[3] == 'd') shfd();
    else if(str[0] == 's' && str[1] == 'h' && str[2] == 'e' && str[3] == 'x') shex();
    else if(str[0] == 'f' && str[1] == 'd' && str[2] == 'l' && str[3] == 't') fdlt();
    else if(str[0] == 'g' && str[1] == 'p' && str[2] == 'l' && str[3] == 't') gplt();
    else if(str[0] == 'g' && str[1] == 't' && str[2] == 'r' && str[3] == 'l') gtrl();
    else if(str[0] == 'g' && str[1] == 't' && str[2] == 'g' && str[3] == 'f') gtgf();
    else if(str[0] == 'm' && str[1] == 'n' && str[2] == 'g' && str[3] == 'l') mngl();
    else if(str[0] == 'k' && str[1] == 'c' && str[2] == 'm' && str[3] == 'b') kcmb();
    else if(str[0] == 'd' && str[1] == 'i' && str[2] == 's' && str[3] == 'g') disg(str);
    else if(str[0] == 'f' && str[1] == 'l' && str[2] == 's' && str[3] == 'z') flsz();
    else if(str[0] == 'g' && str[1] == 'f' && str[2] == 's' && str[3] == 'z') gfsz();
    else if(str[0] == 'r' && str[1] == 'v' && str[2] == 'e' && str[3] == 'd') rved();
    else if(str[0] == 'r' && str[1] == 'v' && str[2] == 'g' && str[3] == 'e') rvge();
    else if(str[0] == 'r' && str[1] == 'v' && str[2] == 'g' && str[3] == 'e') rvge();
    else if(str[0] == 'd' && str[1] == 's' && str[2] == 't' && str[3] == 'y'){
        dsty();
        //return 1;
    }
    else if(str[0] == 'r' && str[1] == 'v' && str[2] == 'l' && str[3] == 'g') {
        rvlg();
        return 1;
    } 
    else if(str[0] == 'f' && str[1] == 'l' && str[2] == 'e' && str[3] == 'd') {
        fled();
        return 1;
    }
    else if(str[0] == 'g' && str[1] == 'f' && str[2] == 'e' && str[3] == 'd') {
        gfed();
        return 1;
    }

    return 0;
}

void handler::jrud(){
    int i = 0;
    while(str[i] != ':') i++;
    std::string uid = str.c_str() + i + 1;
    if(u.jguid(uid)) sendMsg("echo:repeat", sockfd);
    else sendMsg("echo:norepeat", sockfd);
}


void handler::jrnm(){
    //根据用户名发送用户id，若用户不存在则返回“norepeat”
    int i = 0;
    while(str[i] != ':') i++;
    std::string buf = str.c_str() + i + 1;
    sendMsg("echo:"+u.Getuid(buf.c_str()), sockfd);
}

void handler::torgst(){
    //发送用户uid或fail
    sendMsg("echo:"+u.newuser(str), sockfd);
}

void handler::jrem(){
    if(u.RepeatEmail(str.c_str()))
        sendMsg("echo:repeat", sockfd);
    else sendMsg("echo:norepeat", sockfd);
}

void handler::jrne(){
    std::string name,email;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        name.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) email.push_back(str[t]);
    //获得uid
    std::string usuid = u.Getuid(name.c_str());
    //获得用户数据
    user U = u.GetUesr(usuid);

    if(email == U.email) sendMsg("echo:right", sockfd);
    else sendMsg("echo:false", sockfd);
}

void handler::pwlg(){
    std::string name,pwd;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        name.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) pwd.push_back(str[t]);
    LOG_INFO("用户名: " << name);
    // 获得uid
    std::string usuid = u.Getuid(name.c_str());
    LOG_INFO("登录时获取uid: " << usuid);

    //若该用户不存在
    if(usuid == "norepeat"){
        sendMsg("echo:pwdfalse", sockfd);
        return;
    }
    //获得用户数据
    user U = u.GetUesr(usuid);
    PasswordHasher hspwd;
    if(hspwd.verifyPassword(pwd.c_str(), U.pwd.c_str())){
        //用户名密码正确
        //发送json序列
        std::string js = lgok(usuid);
        sendMsg("echo:"+ js , sockfd);
    } else{
        //用户名密码错误
        sendMsg("echo:pwdfalse", sockfd);
    }
}

void handler::fdpd(){
    std::string email,pwd;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        email.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) pwd.push_back(str[t]);
    //拿到用户信息
    std::string id = u.EmailGetuid(email.c_str());
    if(id == "norepeat") LOG_WARN("In fuc fdpd, EmainGetuid return norepeat");
    user us = u.GetUesr(id);
    //删除用户的user:uid->json信息
    u.DELUesr(id);
    //修改用户密码
    us.pwd = pwd;
    std::string js = us.toJson();
    u.setutoj(id, js);
    sendMsg("echo:right", sockfd);
}

void handler::emlg(){
    int i = 0;
    while(str[i] != ':') i++;
    std::string buf = str.c_str() + i + 1;
    std::string argu = u.EmailGetuid(buf.c_str());
    if(argu == "norepeat"){
        LOG_WARN("emlg argu is norepeat!");
        sendMsg("echo:false", sockfd);
        return;
    }
    //发送json序列
    sendMsg("echo:"+lgok(argu), sockfd);
}

std::string handler::lgok(std::string uid){
    //拿到用户信息
    user ud = u.GetUesr(uid);

    time_t now = time(nullptr);
    {
        std::lock_guard<std::mutex> lock(map_mutex);
        uslast_active[uid_to_socket[uid]] = now;  // 插入或更新
    }
    if(ud.stat == "online"){
        //顶号
        sendMsg("lgex:"+ uid, uid_to_socket[uid]);
    }
    //切换在线状态
    ud.stat = "online";
    //获取json序列
    std::string js = ud.toJson();
    u.setutoj(uid, js);
    //将套接字存入map
    uid_to_socket[uid] = sockfd;
    socket_to_uid[sockfd] = uid;
    return js;
}

void handler::unlg(std::string buf){
    //拿到名字字符串
    int i = 0;
    while(buf[i] != ':') i++;
    std::string name = buf.c_str() + i + 1;
    //拿到uid
    std::string uid = u.Getuid(name.c_str());
    if(uid == "norepeat") return;
    uulg(uid);
}

void handler::uulg(std::string uid){
    //拿到用户信息
    user ud = u.GetUesr(uid);
    {
        std::lock_guard<std::mutex> lock(map_mutex);
        uslast_active.erase(uid_to_socket[uid]);
    }
    if(uid_to_socket.count(uid) && uid_to_socket[uid] != sockfd)//处理顶号问题
        return ;
    ud.stat = "offline";
    //将该用户在map存的映射清空
    uid_to_socket.erase(uid);
    socket_to_uid.erase(sockfd);
    fd_write_mutexes.erase(sockfd);
    fd_read_mutexes.erase(sockfd);
    //获取json字符串
    std::string js = ud.toJson();
    u.setutoj(uid, js);
}

void handler::rvlg(){
    //拿到uid字符串
    int i = 0;
    while(str[i] != ':') i++;
    std::string uid = str.c_str() + i + 1;
    if(!uid.size()) return;
    uulg(uid);
}

void handler::adfr(){
    //拿到uid1要加uid2的信息
    std::string uid1,uid2, js;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    user ud = u.GetUesr(uid1), ud2 = u.GetUesr(uid2);
    if(ud2.stat == "destroy"){
        sendMsg("echo:dstry",sockfd);
        return;
    }
    js = u.u_report(uid2);
    if(js == "none"){
        sendMsg("echo:dstry",sockfd);
        return;
    }
    report rpt2 = report::fromJson(js);
    if(rpt2.friendapply.count(ud.name) || ud2.shieldlist.count(uid1)){
        sendMsg("echo:sendd",sockfd);
        return;
    }
    //若1不在2的屏蔽表中，发送申请
    std::string jsend = u.AddFrd(ud.name, uid2);
    sendMsg("echo:right",sockfd);
    if (uid_to_socket.count(uid2)) {
        sendMsg("rept:"+jsend, uid_to_socket[uid2]);
    }
}

void handler::adgp(){
    //拿到uid1要加gid的信息
    std::string uid1,gid;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) gid.push_back(str[t]);
    group gp = group::fromJson(u.GetGroup(gid));
    if(gp.owner == "0"){
        sendMsg("echo:disgp", sockfd);
        return;
    }
    if(gp.owner == uid1 || gp.managelist.count(uid1) || gp.memberlist.count(uid1)){//已经在群内
        sendMsg("echo:ingrp", sockfd);
        return;
    }
    //给群主和管理员广播一次
    report rpt = report::fromJson(u.u_report(gp.owner));
    user ud = u.GetUesr(uid1);
    rpt.groupapply.insert(ud.name+":"+gp.name);
    u.svreport( gp.owner, rpt.toJson());
    if(uid_to_socket.count(gp.owner)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[gp.owner]);
    for(std::string str : gp.managelist){
        rpt = report::fromJson(u.u_report(str));
        rpt.groupapply.insert(ud.name+":"+gp.name);
        u.svreport( str, rpt.toJson());
        if(uid_to_socket.count(str)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[str]);
    }
    sendMsg("echo:right", sockfd);
}

void handler::gtrp(){
    //拿到uid字符串
    int i = 0;
    while(str[i] != ':') i++;
    std::string uid = str.c_str() + i + 1;
    //拿到json/none
    std::string js = u.u_report(uid);
    sendMsg("echo:"+js, sockfd);
}

void handler::gtus(){
    //拿到uid字符串
    int i = 0;
    while(str[i] != ':') i++;
    std::string uid = str.c_str() + i + 1;
    //拿到json/none
    user us = u.GetUesr(uid);
    sendMsg("echo:"+us.toJson(), sockfd);
}

void handler::gtgp(){
    //拿到gid字符串
    int i = 0;
    while(str[i] != ':') i++;
    std::string gid = str.c_str() + i + 1;
    //拿到json/norepeat
    std::string sd = u.GetGroup(gid);
    sendMsg("echo:"+sd, sockfd);
}


void handler::jrfd(){
    std::string uid1,uid2;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    user ud1 = u.GetUesr(uid1);
    if(ud1.friendlist.count(uid2)) sendMsg("echo:right", sockfd);
    else sendMsg("echo:false", sockfd);
}



void handler::adfok(){
    int i = 0;
    while(str[i] != '(') i++;
    //获取同意/拒绝标志
    char flag = str[i+1];
    //获取用户名1和uid2
    std::string name1,uid2,uid1,js;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        name1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    //清理ud2的report，并保存
    js = u.u_report(uid2);
    if(js == "none"){
        LOG_INFO("In fuc adfok uid2:" << uid2 << " report return none");
        sendMsg("echo:false", sockfd);
        return;
    }
    report rpt2 = report::fromJson(js);
    rpt2.friendapply.erase(name1);
    u.svreport(uid2, rpt2.toJson());
    //拿到uid1和ud2
    uid1 = u.Getuid(name1.c_str());
    if(uid1 == "norepeat"){
        sendMsg("echo:dstry", sockfd);
        return;
    }
    user ud1 = u.GetUesr(uid1), ud2 = u.GetUesr(uid2);
    if(ud1.stat == "destroy" || ud2.stat == "destroy"){
        sendMsg("echo:dstry", sockfd);
        return;
    }
    if(flag == 'y'){
        //获取u1和u2的用户数据
        ud1.friendlist.insert(uid2);
        ud2.friendlist.insert(uid1);
        //保存ud1、ud2
        u.setutoj(uid1, ud1.toJson());
        u.setutoj(uid2, ud2.toJson());        
    }
    sendMsg("echo:right", sockfd);
    //获取u1的report
    js = u.u_report(uid1);
    if(js == "none"){
        LOG_INFO("In fuc adfok uid1:" << uid1 << " report return none");
        sendMsg("echo:false", sockfd);
        return;
    }
    report rpt1 = report::fromJson(js);

    //若用户2对用户1有好友申请，清除
    if(rpt1.friendapply.count(ud2.name)) rpt1.friendapply.erase(ud2.name);

    //给u1发成功添加好友的notice
    if(flag == 'y')
        rpt1.notice.insert( rpt1.notice.begin(), std::string("adfy(n)") + ud2.name);
    else    
        rpt1.notice.insert( rpt1.notice.begin() ,std::string("adfn(n)") + ud2.name);
    //保存rpt1
    u.svreport(uid1, rpt1.toJson());
    //若uid1在线，发通知
    if(uid_to_socket.count(uid1)) {
        sendMsg("rept:"+rpt1.toJson(), uid_to_socket[uid1]);
        sendMsg("user:"+ud1.toJson(), uid_to_socket[uid1]);
    }
    //给用户2发用户信息，避免本地内容被通信覆盖
    sendMsg("user:"+ud2.toJson(), sockfd);
}


void handler::rdnt(){
    std::string uid,notice;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) notice.push_back(str[t]);
    std::string js = u.u_report(uid);
    if(js == "none"){
        LOG_INFO("In fuc adfok uid1:" << uid << " report return none");
        sendMsg("echo:false", sockfd);
        return;
    }
    report rpt = report::fromJson(js);
    for(i = 0; i < rpt.notice.size();i++)
        if(rpt.notice[i] == notice){
            rpt.notice[i][5] = 'y';
            break;
        }
    u.svreport(uid, rpt.toJson());
    sendMsg("echo:right", sockfd);

}


void handler::rmnt(){
    std::string uid,notice;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) notice.push_back(str[t]);
    std::string js = u.u_report(uid);
    if(js == "none"){
        LOG_INFO("In fuc adfok uid1:" << uid << " report return none");
        sendMsg("echo:false", sockfd);
        return;
    }
    report rpt = report::fromJson(js);
    for(i = 0; i < rpt.notice.size();i++)
        if(rpt.notice[i] == notice){
            rpt.notice.erase(rpt.notice.begin() + i);
            break;
        }
    u.svreport(uid, rpt.toJson());
    sendMsg("echo:right", sockfd);
}

//uid1删uid2
void handler::rmfd(){
    //拿到数据
    std::string uid1,uid2;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    //获取user
    user ud1 = u.GetUesr(uid1), ud2 = u.GetUesr(uid2);
    ud1.friendlist.erase(uid2);
    ud2.friendlist.erase(uid1);
    std::string js = ud2.toJson();
    u.setutoj(uid1, ud1.toJson());
    u.setutoj(uid2, js);
    if(uid_to_socket.count(uid2)) sendMsg("user:"+ js, uid_to_socket[uid2]);
    sendMsg("echo:right", sockfd);
}

void handler::rmgp(std::string &handstr, int fg){
    //拿到数据
    std::string uid1,gid;
    int i = 0;
    while(handstr[i] != ':') i++;
    int j = i + 1;
    while(handstr[j] != ':') {
        uid1.push_back(handstr[j]);
        j++;
    }
    for(int t = j + 1; t < handstr.size(); t++) gid.push_back(handstr[t]);
    user ud1 = u.GetUesr(uid1);
    group gp = group::fromJson(u.GetGroup(gid));
    if(ud1.uid == gp.owner){
        //调用解散函数
        std::string argstr = "disg:"+gid;
        disg(argstr, fg);
        return;
    }
    //1.去掉ud1群列表中的该组，给ud1发user -> 2.去掉群列表的该用户 -> 3.给全体成员广播该退群通知？
    ud1.grouplist.erase(gid);
    u.setutoj(uid1, ud1.toJson());
    if(uid_to_socket.count(uid1)) sendMsg("user:"+ ud1.toJson(), sockfd);
    //去掉群中的该用户
    for(std::string p : gp.managelist){
        if(p == uid1){
            gp.managelist.erase(uid1);
            break;
        }
    }
    for(std::string p : gp.memberlist){
        if(p == uid1){
            gp.memberlist.erase(uid1);
            break;
        }
    }
    u.setgtoj( gid, gp.toJson());
    // char result[512];
    // //exgp(n)uid1:gid
    // sprintf( result, "exgp(n)%s:%s", uid1.c_str(), gid.c_str());
    if(fg == 1)
        sendMsg("echo:right", sockfd);
}



void handler::gtud(){
    int i = 0;
    while(str[i] != ':') i++;
    std::string name = str.c_str() + i + 1;
    std::string sd = u.Getuid(name.c_str());
    if(sd == "norepeat") sendMsg("echo:false", sockfd);
    else sendMsg("echo:"+sd, sockfd);
}

void handler::gtnm(){
    int i = 0;
    while(str[i] != ':') i++;
    std::string uid = str.c_str() + i + 1;
    user ud = u.GetUesr(uid);
    sendMsg("echo:"+ud.name, sockfd);
}
void handler::ggnm(){
    int i = 0;
    while(str[i] != ':') i++;
    std::string gid = str.c_str() + i + 1;
    group ud = group::fromJson(u.GetGroup(gid));
    sendMsg("echo:"+ud.name, sockfd);
}


void handler::ctms(){
    //拿到数据
    std::string uid1,uid2;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    messages msg(u.lrange(uid1, uid2, 0, 13));
    //发给用户
    sendMsg("echo:"+ msg.toJson(), sockfd);
}

void handler::gctm(){
    int i = 0;
    while(str[i] != ':') i++;
    std::string gid = str.c_str() + i + 1;
    messages msg(u.glrange(gid, 0, 13));
    sendMsg("echo:"+ msg.toJson(), sockfd);
}


//处理用户发送消息的请求
//"sdms:"+message
//redis  chat:uid1:uid2 表示uid1收到uid2的消息表
void handler::sdms(){
    std::string msg;
    int i = 0, j;
    while(str[i] != ':') i ++;
    msg = str.substr(i + 1);
    //先判断是不是好友
    message sendm;
    try {
        sendm = message::fromJson(msg);
        // 继续处理
    } catch(const std::exception& e) {
        LOG_ERROR("message fromjson error !!! msg: " << msg.data());
        sendMsg("ctsp:nofrd", sockfd);
        return;
    }
    user ud1 = u.GetUesr(sendm.sender_uid);
    user ud2 = u.GetUesr(sendm.receiver_uid);
    if(ud1.friendlist.count(sendm.receiver_uid) == 0){//说明不为好友
        sendMsg("ctsp:nofrd", sockfd);
        return;
    }
    //再判断是否被屏蔽
    if(ud2.shieldlist.count(ud1.uid)){//被屏蔽，消息记录为单向，不给接受者发通知（x）
        //被屏蔽，提示被屏蔽，消息无法发送
        sendMsg("ctsp:sheld", sockfd);
        return;
    }
    //存入数据库
    u.savechat(msg);
    //同时给uid2存入他的消息表
    u.savechat2(msg);
    //编辑接受者的通知
    report rpt = report::fromJson(u.u_report(sendm.receiver_uid));
    int cnt = rpt.chatfriend[sendm.sender_uid];
    rpt.chatfriend[sendm.sender_uid] = cnt + 1;
    rpt.total_friend_msg++;
    u.svreport(sendm.receiver_uid, rpt.toJson());
    //若接受者在线，给接受者发通知：有新消息
    if(uid_to_socket.count(sendm.receiver_uid)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[sendm.receiver_uid]);
    //给接受者发送这条消息
    if(uid_to_socket.count(sendm.receiver_uid)) sendMsg("chat:"+sendm.toJson(), uid_to_socket[sendm.receiver_uid]);
    //给发送者发回声
    // sendMsg("ctsp:right", sockfd);
}

void handler::sdgm(){
    std::string msg;
    int i = 0, j;
    while(str[i] != ':') i ++;
    msg = str.c_str() + i + 1;
    //先判断在不在群
    message sendm = message::fromJson(msg);
    user ud1 = u.GetUesr(sendm.sender_uid);
    std::string js = u.GetGroup(sendm.receiver_uid);
    if(js == "norepeat"){
        sendMsg("ctsp:nofrd", sockfd);
        return;
    }
    group gp = group::fromJson(js);
    if(gp.owner == "0"){
        sendMsg("ctsp:disgp", sockfd);
        return;
    }
    bool exist = false;
    if(ud1.uid == gp.owner) exist = true;
    if(gp.managelist.count(ud1.uid)) exist = true;
    if(gp.memberlist.count(ud1.uid)) exist = true;
    if(!exist){//说明不在群内
        sendMsg("ctsp:nofrd", sockfd);
        return;
    }
    //存入数据库
    u.savegchat(msg);
    //编辑接受者的通知
    report rpt;

    if(sendm.sender_uid != gp.owner){
        rpt = report::fromJson(u.u_report(gp.owner));
        int cnt = rpt.chatgroup[gp.gid];
        rpt.chatgroup[gp.gid] = cnt + 1;
        rpt.total_group_msg++;
        u.svreport(gp.owner, rpt.toJson());
        //若接受者在线，给接受者发通知：有新消息
        if(uid_to_socket.count(gp.owner)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[gp.owner]);
        //给接受者发送这条消息
        if(uid_to_socket.count(gp.owner)) sendMsg("chat:"+sendm.toJson(), uid_to_socket[gp.owner]);
    }
    for(std::string uid2 : gp.managelist){
        if(uid2 != sendm.sender_uid){
            rpt = report::fromJson(u.u_report(uid2));
            int cnt = rpt.chatgroup[gp.gid];
            rpt.chatgroup[gp.gid] = cnt + 1;
            rpt.total_group_msg++;
            u.svreport(uid2, rpt.toJson());
            //若接受者在线，给接受者发通知：有新消息
            if(uid_to_socket.count(uid2)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[uid2]);
            //给接受者发送这条消息
            if(uid_to_socket.count(uid2)) sendMsg("chat:"+sendm.toJson(), uid_to_socket[uid2]);
        }
    }
    for(std::string uid2 : gp.memberlist){
        if(uid2 != sendm.sender_uid){
            rpt = report::fromJson(u.u_report(uid2));
            int cnt = rpt.chatgroup[gp.gid];
            rpt.chatgroup[gp.gid] = cnt + 1;
            rpt.total_group_msg++;
            u.svreport(uid2, rpt.toJson());
            //若接受者在线，给接受者发通知：有新消息
            if(uid_to_socket.count(uid2)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[uid2]);
            //给接受者发送这条消息
            if(uid_to_socket.count(uid2)) sendMsg("chat:"+sendm.toJson(), uid_to_socket[uid2]);
        }
    }
    
    //给发送者发回声
    //sendMsg("ctsp:right", sockfd);
}

void handler::rdpg(){
    //拿到数据
    std::string uid1,uid2;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    int t = u.llen(uid1, uid2);
    char arr[512];
    sprintf(arr, "echo:%d", t);
    sendMsg(arr, sockfd);
}
void handler::rdgc(){
    //拿到数据
    int i = 0;
    while(str[i] != ':') i++;
    std::string gid = str.c_str() + i + 1;
    int t = u.gllen(gid);
    char arr[512];
    sprintf(arr, "echo:%d", t);
    sendMsg(arr, sockfd);
}

void handler::svrp(){
    //拿到数据
    std::string uid1,sd;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) sd.push_back(str[t]);
    u.svreport(uid1, sd);
    sendMsg("echo:right", sockfd);
}

void handler::ndms(){
    //拿到数据
    std::string uid1, uid2, msgcnt;
    int i = 0, t, cnt;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    t = j + 1;
    while(str[t] != ':') {
        uid2.push_back(str[t]);
        t++;
    }
    for(int k = t + 1; k < str.size(); k++) msgcnt.push_back(str[k]);
    sscanf(msgcnt.c_str(), "%d", &cnt);
    messages msgs = u.lrange(uid1, uid2, cnt, cnt+6);
    sendMsg("echo:"+ msgs.toJson(), sockfd);
}

void handler::ndgm(){
    //拿到数据
    std::string gid,msgcnt;
    int i = 0, cnt;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        gid.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) msgcnt.push_back(str[t]);
    sscanf(msgcnt.c_str(), "%d", &cnt);
    messages msgs = u.glrange( gid, cnt, cnt+6);
    sendMsg("echo:"+ msgs.toJson(), sockfd);
}

//uid1屏蔽uid2
void handler::shfd(){
    //拿到数据
    std::string uid1,uid2;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    //在uid1的屏蔽表添加uid2
    user ud1 = u.GetUesr(uid1);
    if(ud1.shieldlist.count(uid2)){
        sendMsg("echo:sheld", sockfd);
        return;
    }
    ud1.shieldlist.insert(uid2); 
    //保存屏蔽结果
    u.setutoj( uid1, ud1.toJson());
    sendMsg("echo:right", sockfd);
}


void handler::shex(){
    //拿到数据
    std::string uid1,uid2;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    //在uid1的屏蔽表添加uid2
    user ud1 = u.GetUesr(uid1);
    ud1.shieldlist.erase(uid2); 
    //保存屏蔽结果
    u.setutoj( uid1, ud1.toJson());
    sendMsg("echo:right", sockfd);
}

void handler::jrgn(){
    //根据群聊名发送gid，若群名不存在则返回“norepeat”
    int i = 0;
    while(str[i] != ':') i++;
    std::string buf = str.c_str() + i + 1;
    sendMsg("echo:"+u.Getgid(buf.c_str()), sockfd);
}


void handler::ctgp(){
    //发送群组uid或fail
    sendMsg("echo:"+u.newgroup(str), sockfd);
}

void handler::fdlt(){
    //拿到数据
    std::string uid1,fdname;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) fdname.push_back(str[t]);
    user ud1 = u.GetUesr(uid1);

    friendnamelist fnl;
    for(std::string uid2 : ud1.friendlist){
        user ud2 = u.GetUesr(uid2);
        if(ud2.name.find(fdname) != -1)
            fnl.data.push_back(ud2.uid);
    }
    sendMsg("echo:"+fnl.toJson(), sockfd);
}

//adg(y:uname:gname:handler(用户加群),改数据库
void handler::adgok(){
    int i = 6;
    char input = str[4];
    std::string uname, gname, handname;
    while(str[i] != ':') uname.push_back(str[i++]);
    i++;
    while(str[i] != ':') gname.push_back(str[i++]);
    handname = str.c_str() + i + 1;
    std::string uid = u.Getuid(uname.c_str()), gid = u.Getgid(gname.c_str());
    group gp = group::fromJson(u.GetGroup(gid));
    if(uid == "norepeat"){
        std::string apply = uname+":"+gname;
        report rpt = report::fromJson(u.u_report(gp.owner));
        rpt.groupapply.erase(apply);
        u.svreport( gp.owner, rpt.toJson());

        for(std::string mvs : gp.managelist){
            rpt = report::fromJson(u.u_report(mvs));
            rpt.groupapply.erase(apply);
            u.svreport( mvs, rpt.toJson());
            if(uid_to_socket.count(mvs)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[mvs]);
        }

        sendMsg("echo:dstry", sockfd);
        return;
    }
    //同意：1.用户结构体中群组列表加入该群gid -> 2.该群成员中加入该用户 -> 3.去除群主管理员通知中这条申请，给他们发结果通知 ->
    //拒绝: 去除群主管理员通知中这条申请，给他们发结果通知
    user ud = u.GetUesr(uid);
    if(gp.owner == "0"){
        sendMsg("echo:disgp", sockfd);
        return;
    }
    if(ud.stat == "destroy"){
        std::string apply = uname+":"+gname;
        report rpt = report::fromJson(u.u_report(gp.owner));
        rpt.groupapply.erase(apply);
        u.svreport( gp.owner, rpt.toJson());

        for(std::string mvs : gp.managelist){
            rpt = report::fromJson(u.u_report(mvs));
            rpt.groupapply.erase(apply);
            u.svreport( mvs, rpt.toJson());
            if(uid_to_socket.count(mvs)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[mvs]);
        }

        rpt = report::fromJson(u.u_report(uid));
        u.svreport( uid, rpt.toJson());
        if(uid_to_socket.count(uid)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[uid]);
        sendMsg("echo:dstry", sockfd);
        return;
    }
    if(gp.owner == uid || gp.managelist.count(uid) || gp.memberlist.count(uid)){//已经在群内
        sendMsg("echo:ingrp", sockfd);
        return;
    }
    if(input == 'Y' || input == 'y'){
        //用户结构体中群组列表加入该群gid
        ud.grouplist.insert(gid);
        u.setutoj(uid, ud.toJson());
        //该群成员中加入该用户
        gp.memberlist.insert(uid);
        u.setgtoj(gid, gp.toJson());
    }
    //拼接通知
    char result[512];
    sprintf( result, "adg%c(n)%s:%s:%s", input, uname.c_str(), gname.c_str(), handname.c_str());
    std::string apply = uname+":"+gname;
    //去除群主通知中这条申请
    report rpt = report::fromJson(u.u_report(gp.owner));
    rpt.groupapply.erase(apply);
    //替换群主的通知
    rpt.notice.insert(rpt.notice.begin(), result);
    u.svreport( gp.owner, rpt.toJson());
    //若在线，提示该通知
    if(uid_to_socket.count(gp.owner)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[gp.owner]);
    //去除管理员通知中这条申请
    for(std::string mvs : gp.managelist){
        rpt = report::fromJson(u.u_report(mvs));
        rpt.groupapply.erase(apply);
        rpt.notice.insert(rpt.notice.begin(), result);
        u.svreport( mvs, rpt.toJson());
        if(uid_to_socket.count(mvs)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[mvs]);
    }
    sendMsg("echo:right", sockfd);
    //给对应用户发送user提醒和可能的通知
    if(uid_to_socket.count(uid)) sendMsg("user:"+ud.toJson(), uid_to_socket[uid]);
    rpt = report::fromJson(u.u_report(uid));
    rpt.notice.insert(rpt.notice.begin(), result);
    u.svreport( uid, rpt.toJson());
    if(uid_to_socket.count(uid)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[uid]);
}

void handler::gplt(){
    //根据gid获取，若群名不存在则返回“norepeat”
    int i = 0;
    while(str[i] != ':') i++;
    std::string gid = str.c_str() + i + 1;
    std::string js = u.GetGroup(gid);
    if(js == "norepeat"){
        sendMsg("echo:norepeat", sockfd);
        return;
    }
    group gp = group::fromJson(js);
    if(gp.owner == "0"){
        sendMsg("echo:disgp", sockfd);
        return;
    }
    friendnamelist fnl;
    fnl.data.push_back(gp.owner);
    for(std::string tmp : gp.managelist) fnl.data.push_back(tmp);
    for(std::string tmp : gp.memberlist) fnl.data.push_back(tmp);
    sendMsg("echo:"+fnl.toJson(), sockfd);
}

void handler::mngl(){
    int i = 0;
    while(str[i] != ':') i++;
    std::string uid = str.c_str() + i + 1, js;
    user ud = u.GetUesr(uid);
    friendnamelist gnl;
    group gp;
    for(std::string gid : ud.grouplist){
        js = u.GetGroup(gid);
        if(js == "norepeat") continue;
        gp = group::fromJson(js);
        if(gp.owner == uid || gp.managelist.count(uid)) gnl.data.push_back(gp.gid);
    }
    sendMsg("echo:"+gnl.toJson(), sockfd);
}

void handler::admn(){
    //拿到数据
    std::string gid,uid2,js;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        gid.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    user ud = u.GetUesr(uid2);
    js = u.GetGroup(gid);
    if(js == "norepeat"){
        sendMsg("echo:false", sockfd);
        return;
    }
    group gp = group::fromJson(js);
    if(gp.owner == "0"){
        sendMsg("echo:disgp", sockfd);
        return;
    }
    gp.memberlist.erase(uid2);
    gp.managelist.insert(uid2);
    u.setgtoj( gid, gp.toJson());
    js = u.u_report(uid2);
    if(js == "none"){
        sendMsg("echo:false", sockfd);
        return;
    }
    //拼接通知
    char result[512];
    sprintf( result, "admy(n)%s:%s", gp.name.c_str(), ud.name.c_str());
    for(std::string u2 : gp.managelist){
        js = u.u_report(u2);
        if(js == "none"){
            sendMsg("echo:false", sockfd);
            return;
        }
        report rpt = report::fromJson(js);
        rpt.notice.insert(rpt.notice.begin(), std::string(result));
        u.svreport( u2, rpt.toJson());
        if(uid_to_socket.count(u2) && u2 != uid2) sendMsg("rept:"+rpt.toJson(), uid_to_socket[u2]);
    }
    for(std::string u2 : gp.memberlist){
        js = u.u_report(u2);
        if(js == "none"){
            sendMsg("echo:false", sockfd);
            return;
        }
        report rpt = report::fromJson(js);
        rpt.notice.insert(rpt.notice.begin(), std::string(result));
        u.svreport( u2, rpt.toJson());
        if(uid_to_socket.count(u2) && u2 != uid2) sendMsg("rept:"+rpt.toJson(), uid_to_socket[u2]);
    }
    if(uid_to_socket.count(uid2)) sendMsg("rept:modmanage", uid_to_socket[uid2]);
    sendMsg("echo:right", sockfd);
}

void handler::dlmn(){
    //拿到数据
    std::string gid,uid2,js;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        gid.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) uid2.push_back(str[t]);
    user ud = u.GetUesr(uid2);
    js = u.GetGroup(gid);
    if(js == "norepeat"){
        sendMsg("echo:false", sockfd);
        return;
    }
    group gp = group::fromJson(js);
    if(gp.owner == "0"){
        sendMsg("echo:disgp", sockfd);
        return;
    }
    gp.managelist.erase(uid2);
    gp.memberlist.insert(uid2);
    u.setgtoj( gid, gp.toJson());
    js = u.u_report(uid2);
    if(js == "none"){
        sendMsg("echo:false", sockfd);
        return;
    }
    //拼接通知
    char result[512];
    sprintf( result, "admn(n)%s:%s", gp.name.c_str(), ud.name.c_str());
    for(std::string u2 : gp.managelist){
        js = u.u_report(u2);
        if(js == "none"){
            sendMsg("echo:false", sockfd);
            return;
        }
        report rpt = report::fromJson(js);
        rpt.notice.insert(rpt.notice.begin(), std::string(result));
        u.svreport( u2, rpt.toJson());
        if(uid_to_socket.count(u2) && u2 != uid2) sendMsg("rept:"+rpt.toJson(), uid_to_socket[u2]);
    }
    for(std::string u2 : gp.memberlist){
        js = u.u_report(u2);
        if(js == "none"){
            sendMsg("echo:false", sockfd);
            return;
        }
        report rpt = report::fromJson(js);
        rpt.notice.insert(rpt.notice.begin(), std::string(result));
        u.svreport( u2, rpt.toJson());
        if(uid_to_socket.count(u2) && u2 != uid2) sendMsg("rept:"+rpt.toJson(), uid_to_socket[u2]);
    }
    if(uid_to_socket.count(uid2)) sendMsg("rept:modmanage", uid_to_socket[uid2]);
    sendMsg("echo:right", sockfd);
}

//"kcmb:"+gid+":"+ uid2+":"+handleuid
void handler::kcmb(){
    int i = 5;
    char input = str[4];
    std::string gid, uid2, handuid, js;
    while(str[i] != ':') gid.push_back(str[i++]);
    i++;
    while(str[i] != ':') uid2.push_back(str[i++]);
    handuid = str.c_str() + i + 1;
    js = u.GetGroup(gid);
    if(js == "norepeat"){
        LOG_WARN("gid: " << gid);
        sendMsg("echo:false", sockfd);
        return;
    }
    group gp = group::fromJson(js);
    if(gp.owner == "0"){
        sendMsg("echo:right", sockfd);
        return;
    }
    user ud1 = u.GetUesr(handuid) ,ud2 = u.GetUesr(uid2);
    if(gp.owner != handuid && gp.managelist.count(handuid) == 0){
        sendMsg("echo:nopms", sockfd);
        return;
    }
    if(handuid != gp.owner){
        if(uid2 == gp.owner || gp.managelist.count(uid2)){
            sendMsg("echo:pmser", sockfd);
            return;
        }
    }
    ud2.grouplist.erase(gid);
    u.setutoj(uid2, ud2.toJson());
    if(uid_to_socket.count(uid2)) sendMsg("user:"+ ud2.toJson(), uid_to_socket[uid2]);
    //去掉群中的该用户
    for(std::string p : gp.managelist){
        if(p == uid2){
            gp.managelist.erase(uid2);
            break;
        }
    }
    for(std::string p : gp.memberlist){
        if(p == uid2){
            gp.memberlist.erase(uid2);
            break;
        }
    }
    u.setgtoj( gid, gp.toJson());
    //命令"kcmb(n)%s:%s:%s", input, uname.c_str(), gname.c_str(), handname.c_str()
    char result[512];
    sprintf( result, "kcmb(n)%s:%s:%s", ud2.name.c_str(), gp.name.c_str(), ud1.name.c_str());

    //广播
    report rpt = report::fromJson(u.u_report(gp.owner));
    rpt.notice.insert(rpt.notice.begin(), result);
    u.svreport(gp.owner, rpt.toJson());
    if(uid_to_socket.count(gp.owner)) sendMsg("rept:"+rpt.toJson() , uid_to_socket[gp.owner]);

    rpt = report::fromJson(u.u_report(uid2));
    rpt.notice.insert(rpt.notice.begin(), result);
    u.svreport(uid2, rpt.toJson());
    if(uid_to_socket.count(uid2)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[uid2]);

    for(std::string p : gp.managelist){
        rpt = report::fromJson(u.u_report(p));
        rpt.notice.insert(rpt.notice.begin(), result);
        u.svreport(p, rpt.toJson());
        if(uid_to_socket.count(p)) sendMsg("rept:"+rpt.toJson() , uid_to_socket[p]);
    }
    for(std::string p : gp.memberlist){
        rpt = report::fromJson(u.u_report(p));
        rpt.notice.insert(rpt.notice.begin(), result);
        u.svreport(p, rpt.toJson());
        if(uid_to_socket.count(p)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[p]);
    }
    
    //给发送者发回声
    sendMsg("echo:right", sockfd);
}

void handler::disg(std::string& handstr, int fg){
    //拿到数据
    int i = 0;
    while(handstr[i] != ':') i++;
    std::string gid = handstr.c_str() + i + 1, js;

    js = u.GetGroup(gid);
    if(js == "norepeat"){
        LOG_WARN("gid: " << gid);
        sendMsg("echo:false", sockfd);
        return;
    }
    group gp = group::fromJson(js);

    //解析命令"disg(n)%s", gname.c_str()
    char result[512];
    sprintf( result, "disg(n)%s", gp.name.c_str());
    //删gchat -> owner设置为0标记注销 -> 清groupname键值对
    //删所有成员列表中的该群，并更新在线时的user，发群聊解散通知 -> 清用户report未读群聊消息表中对应群的消息
    u.disbandgroup( gp.gid, gp.name);

    //群主
    user ud = u.GetUesr(gp.owner);
    ud.grouplist.erase(gp.gid);
    u.setutoj( gp.owner, ud.toJson());
    if(uid_to_socket.count(gp.owner)) sendMsg("user:"+ud.toJson(), uid_to_socket[gp.owner]);
    report rpt = report::fromJson(u.u_report(gp.owner));
    rpt.total_group_msg -= rpt.chatgroup[gp.gid];
    rpt.chatgroup.erase(gp.gid);
    rpt.notice.insert(rpt.notice.begin(), result);
    u.svreport( gp.owner, rpt.toJson());
    if(uid_to_socket.count(gp.owner)) sendMsg("rept:disg", uid_to_socket[gp.owner]);
    gp.owner = "0";
    u.setgtoj( gp.gid, gp.toJson());

    for(std::string uid : gp.memberlist){
        ud = u.GetUesr(uid);
        ud.grouplist.erase(gp.gid);
        u.setutoj( uid, ud.toJson());
        if(uid_to_socket.count(uid)) sendMsg("user:"+ud.toJson(), uid_to_socket[uid]);
        rpt = report::fromJson(u.u_report(uid));
        rpt.total_group_msg -= rpt.chatgroup[gp.gid];
        rpt.chatgroup.erase(gp.gid);
        rpt.notice.insert(rpt.notice.begin(), result);
        u.svreport( uid, rpt.toJson());
        if(uid_to_socket.count(uid)) sendMsg("rept:disg", uid_to_socket[uid]);
    }

    for(std::string uid : gp.managelist){
        ud = u.GetUesr(uid);
        ud.grouplist.erase(gp.gid);
        u.setutoj( uid, ud.toJson());
        if(uid_to_socket.count(uid)) sendMsg("user:"+ud.toJson(), uid_to_socket[uid]);
        rpt = report::fromJson(u.u_report(uid));
        rpt.total_group_msg -= rpt.chatgroup[gp.gid];
        rpt.chatgroup.erase(gp.gid);
        rpt.notice.insert(rpt.notice.begin(), result);
        u.svreport( uid, rpt.toJson());
        if(uid_to_socket.count(uid)) sendMsg("rept:disg", uid_to_socket[uid]);
    }
    if(fg == 1)
        sendMsg("echo:right", sockfd);
}


void handler::gtfi(){
    //拿到数据
    std::string uid,path;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) path.push_back(str[t]);
    sendMsg("echo:"+u.Getfid(uid, path, false), sockfd);
}

std::string GetFileName(const char arr[]){
  int i = strlen(arr) - 1;
  std::string str;
  for(;i >= 0; i--){
    if(arr[i] == '/') break;
    else str.push_back(arr[i]);
  }
  std::reverse(str.begin(), str.end());
  return str;
}

void rvfl(std::string &str){
    //拿到数据
    int i = 0;
    while(str[i] != ':') i++;
    std::string packet = str.substr(i + 1);
    LOG_WARN("rvfl内, str.size(): " << str.size() << ", packet.size(): " << packet.size());
    LOG_WARN("rvfl内, str: " << str << ", packet: " << packet);

    if (packet.size() < sizeof(uint32_t)) {
        LOG_WARN("数据包太短，无法解析");
        return;
    }
    //1. 提取前4字节，得到 JSON 字符串长度
    uint32_t json_len;
    std::memcpy(&json_len, packet.data(), sizeof(uint32_t));
    json_len = ntohl(json_len);  // 网络字节序转主机序
    //2. 检查总长度是否合法
    if (packet.size() < sizeof(uint32_t) + json_len) {
        LOG_WARN("数据包长度不足，缺失 JSON 部分");
        return;
    }
    //3. 提取 JSON 字符串
    std::string json_str = packet.substr(sizeof(uint32_t), json_len);
    //4. 解析 JSON
    file_block fb = file_block::fromJson(json_str);
    //5. 提取数据部分
    size_t data_offset = sizeof(uint32_t) + json_len;
    std::string data = packet.substr(data_offset);

    //构造目录路径: ./recvfile/file_<recver_uid>/
    std::string dir_path = "./recvfile/file_" + fb.receiver_uid;
    std::filesystem::create_directories(dir_path);  // 若已存在不会报错

    //构造文件名: sender_uid:fid:filename
    std::string file_name = fb.sender_uid + ":" + fb.fid + ":" + GetFileName(fb.filename.data());
    std::string full_path = dir_path + "/" + file_name;

    //写入文件
    FILE* f = fopen(full_path.c_str(), "r+b");
    if (!f) {
        f = fopen(full_path.c_str(), "wb");
        if (!f) {
            LOG_ERROR("无法创建文件: " << strerror(errno));
            return;
        }
    }

    fseek(f, fb.offset, SEEK_SET);
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);

    LOG_INFO("写入 " << full_path << "，偏移 " << fb.offset << "，长度 " << data.size() << " 字节");
}

void rvgf(std::string &str){
    //拿到数据
    int i = 0;
    while(str[i] != ':') i++;
    std::string packet = str.substr(i + 1);
    LOG_WARN("rvgf内, str.size(): " << str.size() << ", packet.size(): " << packet.size());
    LOG_WARN("rvgf内, str: " << str << ", packet: " << packet);

    if (packet.size() < sizeof(uint32_t)) {
        LOG_WARN("数据包太短，无法解析");
        return;
    }
    //1. 提取前4字节，得到 JSON 字符串长度
    uint32_t json_len;
    std::memcpy(&json_len, packet.data(), sizeof(uint32_t));
    json_len = ntohl(json_len);  // 网络字节序转主机序
    //2. 检查总长度是否合法
    if (packet.size() < sizeof(uint32_t) + json_len) {
        LOG_WARN("数据包长度不足，缺失 JSON 部分");
        return;
    }
    //3. 提取 JSON 字符串
    std::string json_str = packet.substr(sizeof(uint32_t), json_len);
    //4. 解析 JSON
    file_block fb = file_block::fromJson(json_str);
    //5. 提取数据部分
    size_t data_offset = sizeof(uint32_t) + json_len;
    std::string data = packet.substr(data_offset);

    //构造目录路径: ./recvfile/gfile_<recver_uid>/
    std::string dir_path = "./recvfile/gfile_" + fb.receiver_uid;
    std::filesystem::create_directories(dir_path);  // 若已存在不会报错

    //构造文件名: sender_uid:fid:filename
    std::string file_name = fb.receiver_uid + ":" + fb.fid + ":" + GetFileName(fb.filename.data());
    std::string full_path = dir_path + "/" + file_name;

    //写入文件
    FILE* f = fopen(full_path.c_str(), "r+b");
    if (!f) {
        f = fopen(full_path.c_str(), "wb");
        if (!f) {
            LOG_ERROR("无法创建文件: " << strerror(errno));
            return;
        }
    }

    fseek(f, fb.offset, SEEK_SET);
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);

    LOG_INFO("写入 " << full_path << "，偏移 " << fb.offset << "，长度 " << data.size() << " 字节");
}

void handler::gtrl(){
    //拿到数据
    int i = 0;
    while(str[i] != ':') i++;
    std::string uid = str.substr(i + 1);
    std::string dir_path = "./recvfile/file_" + uid;
    friendnamelist fnl;
    //打开目录
    DIR* dp=opendir(dir_path.c_str());
    struct dirent * di;
    if(dp==NULL){
        sendMsg("echo:nofile", sockfd);
        return;
    }
    //读目录
    while((di=readdir(dp))!=NULL){
        if(di->d_name[0]=='.')continue;
        fnl.data.push_back(di->d_name);
    }
    closedir(dp);
    sendMsg("echo:"+fnl.toJson(), sockfd);
}

void handler::gtgf(){
    //拿到数据
    int i = 0;
    while(str[i] != ':') i++;
    std::string uid = str.substr(i + 1);
    user ud = u.GetUesr(uid);
    friendnamelist fnl;
    for(std::string gid : ud.grouplist){
        std::string dir_path = "./recvfile/gfile_" + gid;
        //打开目录
        DIR* dp=opendir(dir_path.c_str());
        struct dirent * di;
        if(dp==NULL) continue;
        //读目录
        while((di=readdir(dp))!=NULL){
            if(di->d_name[0]=='.')continue;
            fnl.data.push_back(di->d_name);
        }
        closedir(dp);
    }
    if(!fnl.data.size()){
        sendMsg("echo:nofile", sockfd);
        return;
    }
    sendMsg("echo:"+fnl.toJson(), sockfd);
}

void handler::fled(){
    //拿到block的json字符串
    int i = 0;
    while(str[i] != ':') i++;
    std::string json_str = str.c_str() + i + 1, js1, js2, result;
    file_block block = file_block::fromJson(json_str);
    user ud1 = u.GetUesr(block.sender_uid), ud2 = u.GetUesr(block.receiver_uid);
    js1 = u.u_report(ud1.uid);
    js2 = u.u_report(ud2.uid);
    if(js1 == "none" || js2 == "none"){
        LOG_INFO("fled, js1: " << js1 << ", js2: " << js2);
        sendMsg("echo:uidfs", sockfd);
        return;
    }
    report rpt1 = report::fromJson(js1), rpt2 = report::fromJson(js2);
    //构造命令"sdfu(n)%s:%s:%s", uname.c_str(), recvname.c_str(), filename.c_str()
    result = "sdfu(n)"+ud1.name+":"+ud2.name+":"+block.filename;
    rpt1.notice.insert(rpt1.notice.begin(), result);
    rpt2.notice.insert(rpt2.notice.begin(), result);
    u.svreport(ud1.uid, rpt1.toJson());
    u.svreport(ud2.uid, rpt2.toJson());
    //通知rept有变化
    if(uid_to_socket.count(ud1.uid)) sendMsg("rept:"+rpt1.toJson(), uid_to_socket[ud1.uid]);
    if(uid_to_socket.count(ud2.uid)) sendMsg("rept:"+rpt2.toJson(), uid_to_socket[ud2.uid]);
}

void handler::gfed(){
    //拿到block的json字符串
    int i = 0;
    while(str[i] != ':') i++;
    std::string json_str = str.c_str() + i + 1, js1, js2, result;
    file_block block = file_block::fromJson(json_str);
    user ud1 = u.GetUesr(block.sender_uid);
    js2 = u.GetGroup(block.receiver_uid);
    if(js2 == "norepeat"){
        LOG_INFO("gfed, js2: " << js2);
        sendMsg("echo:gidfs", sockfd);
        return;
    }
    group gp = group::fromJson(js2);
    //构造命令"sdfg(n)%s:%s:%s", uname.c_str(), gname.c_str(), filename.c_str()
    result = "sdfg(n)"+ud1.name+":"+gp.name+":"+block.filename;
    //给群聊每个人广播result

    report rpt = report::fromJson(u.u_report(gp.owner));
    rpt.notice.insert(rpt.notice.begin(), result);
    u.svreport(gp.owner, rpt.toJson());
    //若接受者在线，给接受者发通知：有新消息
    if(uid_to_socket.count(gp.owner)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[gp.owner]);
    
    for(std::string uid2 : gp.managelist){
        rpt = report::fromJson(u.u_report(uid2));
        rpt.notice.insert(rpt.notice.begin(), result);
        u.svreport(uid2, rpt.toJson());
        //若接受者在线，给接受者发通知：有新消息
        if(uid_to_socket.count(uid2)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[uid2]);
    }
    for(std::string uid2 : gp.memberlist){
        rpt = report::fromJson(u.u_report(uid2));
        rpt.notice.insert(rpt.notice.begin(), result);
        u.svreport(uid2, rpt.toJson());
        //若接受者在线，给接受者发通知：有新消息
        if(uid_to_socket.count(uid2)) sendMsg("rept:"+rpt.toJson(), uid_to_socket[uid2]);
    }

}

void handler::flsz(){
    //拿到文件名
    int i = 0;
    while(str[i] != ':') i++;
    std::string filename = str.c_str() + i + 1, uid1;
    if(socket_to_uid.count(sockfd)){
        uid1 = socket_to_uid[sockfd];
    } else{
        sendMsg("echo:error", sockfd);
        return;
    }
    std::string dir_path = "./recvfile/file_" + uid1;
    struct stat st;
    std::string full_path = dir_path + "/" + filename;
    if (stat(full_path.c_str(), &st) != 0){
        sendMsg("echo:error", sockfd);
        return;
    }
    char arr[512];
    sprintf(arr, "%ld", st.st_size);
    sendMsg("echo:"+std::string(arr), sockfd);
}

void handler::gfsz(){
    //拿到文件名
    int i = 0;
    while(str[i] != ':') i++;
    std::string filename = str.c_str() + i + 1, gid;
    for(int j = i + 1; str[j] != ':'; j++) gid.push_back(str[j]);
    std::string dir_path = "./recvfile/gfile_" + gid;
    struct stat st;
    std::string full_path = dir_path + "/" + filename;
    if (stat(full_path.c_str(), &st) != 0){
        sendMsg("echo:error", sockfd);
        return;
    }
    char arr[512];
    sprintf(arr, "%ld", st.st_size);
    sendMsg("echo:"+std::string(arr), sockfd);
}

void handler::sdfl(){
    //拿到文件名
    std::string uid1;
    int i = 0, j;
    while(str[i] != ':') i++;
    for(j = i+1; str[j] != ':'; j++) uid1.push_back(str[j]);
    std::string filename = str.c_str() + j + 1;
    std::string uid2, fid, fname;
    int m = 0, t;
    while(str[m] != ':') uid2.push_back(str[m++]);
    j = m + 1;
    while(str[j] != ':') fid.push_back(str[j++]);
    for(int k = j + 1; k < str.size(); k++) fname.push_back(str[k]);
    std::string dir_path = "./recvfile/file_" + uid1;
    std::string path = dir_path + "/" + filename;
    struct stat st;
    int sockfd2 = uid_to_socket[uid1];
    if(lstat(path.c_str(),&st) == -1){//获取文件状态
        LOG_ERROR("lstat error");
        sendMsg("echo:error", sockfd2);
        return ;
    }
    FILE* file = fopen(path.c_str(), "rb");
    if (file == NULL) {
        LOG_ERROR("fopen error");
        sendMsg("echo:error", sockfd2);
        return;
    }
    LOG_INFO("sdfl start");
    sendMsg("echo:start", sockfd2);
    const size_t block_size = 4 * 1024 * 1024; // 4MB
    std::vector<char> buf(block_size); // 改用堆内存避免栈溢出

    long long offset = 0;
    size_t bytesRead;
    file_block block;

    while ((bytesRead = fread(buf.data(), 1, block_size, file)) > 0) {
        // 构造 file_block
        block.sender_uid = uid2;
        block.receiver_uid = uid1;
        block.fid = fid;
        block.filename = fname;
        block.timestamp = file_block::get_beijing_time();
        block.offset = offset;
        block.is_group = false;
        block.is_file = true;

        std::string json_str = block.toJson();
        uint32_t json_len = htonl(json_str.size());

        // 构造完整消息
        std::string packet;
        packet.reserve(sizeof(json_len) + json_str.size() + bytesRead);
        packet.append(reinterpret_cast<const char*>(&json_len), sizeof(json_len));
        packet.append(json_str);
        packet.append(buf.data(), bytesRead);

        LOG_INFO("[" << ++i << "]\njson_len: " << json_str.size() << " packet.size(): " << packet.size() << "\njson_str: " << json_str);


        if (sendFILE(packet, sockfd) == -1) {
            sendMsg("error", sockfd);
            fclose(file);
            return;
        }

        offset += bytesRead;
    }

    fclose(file);
    LOG_INFO("文件内容已经发完");
    sendMsg("end", sockfd);
}

void handler::sdgf(){
    //拿到文件名
    std::string uid1;
    int i = 0, j;
    while(str[i] != ':') i++;
    for(j = i+1; str[j] != ':'; j++) uid1.push_back(str[j]);
    std::string filename = str.c_str() + j + 1, gid;
    for(int j = 0; filename[j] != ':'; j++) gid.push_back(filename[j]);
    std::string fid, fname;
    int m = 0, t;
    j = m + 1;
    while(str[j] != ':') fid.push_back(str[j++]);
    for(int k = j + 1; k < str.size(); k++) fname.push_back(str[k]);
    std::string dir_path = "./recvfile/gfile_" + gid;
    std::string path = dir_path + "/" + filename;
    struct stat st;
    int sockfd2 = uid_to_socket[uid1];
    if (lstat(path.c_str(), &st) == -1) { // 获取文件状态
        LOG_ERROR("lstat error");
        sendMsg("echo:error", sockfd2);
        return;
    }
    FILE* file = fopen(path.c_str(), "rb");
    if (file == NULL) {
        LOG_ERROR("fopen error");
        sendMsg("echo:error", sockfd2);
        return;
    }
    LOG_INFO("sdgf start");
    sendMsg("echo:start", sockfd2);
    const size_t block_size = 4 * 1024 * 1024; // 4MB
    std::vector<char> buf(block_size); // 用堆内存避免栈溢出
    long long offset = 0;
    size_t bytesRead;
    file_block block;

    while ((bytesRead = fread(buf.data(), 1, block_size, file)) > 0) {
        // 构造 file_block
        block.sender_uid = gid;
        block.receiver_uid = uid1;
        block.fid = fid;
        block.filename = filename;
        block.timestamp = file_block::get_beijing_time();
        block.offset = offset;
        block.is_group = true;
        block.is_file = false;

        std::string json_str = block.toJson();
        uint32_t json_len = htonl(json_str.size());

        // 构造完整消息
        std::string packet;
        packet.reserve(sizeof(json_len) + json_str.size() + bytesRead);
        packet.append(reinterpret_cast<const char*>(&json_len), sizeof(json_len));
        packet.append(json_str);
        packet.append(buf.data(), bytesRead);

        LOG_INFO("[" << ++i << "]\njson_len: " << json_str.size()
         << " packet.size(): " << packet.size()
         << "\njson_str: " << json_str);

        if (sendFILE(packet, sockfd) == -1) {
            sendMsg("error", sockfd);
            return;
        }

        offset += bytesRead;
    }

    LOG_INFO("文件内容已经发完");
    sendMsg("end", sockfd);
}

void handler::rved(){
    //拿到数据
    std::string uid1,fname, uid2;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) fname.push_back(str[t]);
    for(int t = 0; fname[t] != ':'; t++) uid2.push_back(fname[t]);
    std::string js = u.u_report(uid1);
    if(js == "none") return ;

    report rpt = report::fromJson(js);
    user ud2 = u.GetUesr(uid2.c_str());
    char report[4096];
    //构造通知"rvfu(n):%s:%s", sendname.c_str(), filename.c_str()
    sprintf( report, "rvfu(n):%s:%s", ud2.name.c_str(), GetFileName(fname.c_str()).c_str());
    rpt.notice.insert(rpt.notice.begin(), report);
    
    u.svreport( uid1, rpt.toJson());
    if(uid_to_socket.count(uid1))
        sendMsg("rept:"+rpt.toJson(), uid_to_socket[uid1]);
}

void handler::rvge(){
    //拿到数据
    std::string uid1,fname, gid;
    int i = 0;
    while(str[i] != ':') i++;
    int j = i + 1;
    while(str[j] != ':') {
        uid1.push_back(str[j]);
        j++;
    }
    for(int t = j + 1; t < str.size(); t++) fname.push_back(str[t]);
    for(int t = 0; fname[t] != ':'; t++) gid.push_back(fname[t]);
    std::string js = u.u_report(uid1);
    if(js == "none") return ;

    report rpt = report::fromJson(js);
    std::string js2 = u.GetGroup(gid.c_str());
    if(js2 == "norepeat") return ;
    group gp = group::fromJson(js2);
    char report[4096];
    //构造通知"rvfu(n):%s:%s", sendname.c_str(), filename.c_str()
    sprintf( report, "rvfg(n):%s:%s", gp.name.c_str(), GetFileName(fname.c_str()).c_str());
    rpt.notice.insert(rpt.notice.begin(), report);
    
    u.svreport( uid1, rpt.toJson());
    if(uid_to_socket.count(uid1))
        sendMsg("rept:"+rpt.toJson(), uid_to_socket[uid1]);
}

//销毁用户
void handler::dsty(){
    int i = 0;
    while(str[i] != ':') i++;
    std::string uid1 = str.c_str() + i + 1, js, uid2;
    user ud1 = u.GetUesr(uid1);
    std::string startname = ud1.name;
    //user内名字加上已注销
    char result[512];
    sprintf(result, "dsty(n):%s", ud1.name.c_str());
    ud1.name += "（已注销）";
    ud1.stat = "destroy";

    //轮询所有好友告知user变化
    for(std::string uid2 : ud1.friendlist){
        user ud2 = u.GetUesr(uid2);
        ud2.friendlist.erase(uid1);
        ud2.shieldlist.erase(uid1);
        js = ud2.toJson();
        u.setutoj( uid2, js);
        if(uid_to_socket.count(uid2)) sendMsg("user:"+ js, uid_to_socket[uid2]);

        js = u.u_report(uid2);
        if(js == "none") continue;
        report rpt2 = report::fromJson(js);
        rpt2.notice.insert(rpt2.notice.begin(), std::string(result));
        js = rpt2.toJson();
        u.svreport( uid2, js);
        if(uid_to_socket.count(uid2)) sendMsg("rept:"+ js, uid_to_socket[uid2]);
    }
    //轮询所有群聊并退出
    for(std::string gid : ud1.grouplist){
        std::string tmp = "rmpg:" + uid1 + ":" + gid;
        rmgp(tmp, 2);
    }

    //username对于uid的映射清除
    //清除该用户id的report
    //清除关于该用户的所有聊天
    //清除email表里该用户的email和email:%s的映射
    u.destroy_user(uid1, startname, ud1.email);
    u.setutoj( uid1, ud1.toJson());
    sendMsg("echo:right", sockfd);
}