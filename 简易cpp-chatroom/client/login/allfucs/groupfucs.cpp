#include "./groupfucs.h"

void groupfucs::addgroup(){
    Client* c = (Client*)clientp;
    Socket* sock = c->getSocket();
    system("clear");
    printf("\033[0;32m已选择加入群聊功能(按ESC可返回上一级菜单)\033[0m\n");
    std::string red;
    char buf[520];
    char gid[256];
    printf("\033[0;32m请输入您要加入群聊的gid:\n\033[0m>");
    while(1){
        chu(gid);
        if(enter(gid, 0) == -1) return ; 
        if(strlen(gid) <= 3){
            printf("\033[0;31m该gid无效，请重新输入。\033[0m\n>");
            continue;
        }
        //判断是否在群
        if(us.grouplist.count(gid)){
            printf("\033[0;31m已在该群内，不能重复加入。\033[0m\n>");
            continue;
        }
        //判断此群是否存在
        sprintf(buf, "gtgp:%s", gid);
        sock->sendMsg(buf);
        red = EchoMsgQueue.wait_and_pop();
        if(red == "norepeat"){
            printf("\033[0;31m该gid无效，请重新输入。\033[0m\n>");
            continue;
        }
        
        chu(buf);
        red.clear();
        printf("\033[0;32m正在发送加群请求...\n\033[0m");
        fflush(stdout); // 手动刷新标准输出缓冲区
        //给服务器处理
        chu(buf);
        sprintf(buf, "adgp:%s:%s",us.uid.c_str(), gid);
        sock->sendMsg(buf);
        red = EchoMsgQueue.wait_and_pop();
        if(red == "right") {
            printf("\033[0;32m加群请求发送成功，请按任意键继续...\n\033[0m");
            char rub;
            rub = charget();
            return;
        } else if(red == "ingrp"){
            printf("\033[0;31m已在该群内，不能重复加入。\033[0m\n>");
            continue;
        } else if(red == "disgp"){
            printf("\033[0;31m该群已注销，无法加入。\033[0m\n>");
            continue;
        } else{
            printf("\033[0;31m\n数据异常，请稍后再试。\033[0m\n");
            continue;
        }
    }
    return ;

}

void groupfucs::creategroup(){
    Client * cp = (Client*)clientp;
    Socket* sock = cp->getSocket();
    std::string str;
    system("clear");
    printf("\033[0;32m请输入您要创建的群聊名:\033[0m\n>");

    while(1){
        //int ret = read(STDIN_FILENO,name,sizeof name);
        chu(name);
        int ret = enter(name, 0);
        if(ret == -1) return;
        //检测空格
        int key = 0;
        for(int i = 0; i < ret; i++){
            if(name[i] == ' ') {
                key = 1;
                break;
            }
        }
        if(key == 1){
            key = 0;
            printf("\033[0;31m群聊名仅允许英文字母、数字和汉字，请重新输入。\n\033[0m>");
            continue;
        }
        //判断群聊名合法性
        int tmp = is_valid_username(name);
        if(tmp == -1){
            printf("\033[0;31m群聊名仅允许英文字母、数字和汉字，请重新输入。\n\033[0m>");
            continue;
        }
        if(tmp <= 3) {
            printf("\033[0;31m群聊名至少为4位，请重新输入。\n\033[0m>");
            continue;
        }
        //判断是否有重名
        //询问服务器
        sock->sendMsg("jrgn:"+std::string(name));//judge_repeat_name
        str = EchoMsgQueue.wait_and_pop();
        if(strcmp(str.c_str(), "norepeat") != 0){
            printf("\033[0;31m群聊名已存在，请重新输入。\n\033[0m>");
            continue;
        }
        //创建该群聊
        sock->sendMsg("ctgp:"+std::string(name)+":"+us.uid);
        str = EchoMsgQueue.wait_and_pop();
        us.grouplist.insert(str);
        break;
    }
    printf("\033[0;32m创建群聊成功!\n\033[0m");
    printf("\033[0;32m群名:\033[0m%s\n", name);
    printf("\033[0;32m群id:\033[0m%s\n", str.c_str());
    printf("\033[0;32m请按任意键继续...\033[0m");
    charget();
}

void groupfucs::list(char c){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if(!us.grouplist.size()){
        printf("\033[0;32m当前没有加入群组。\n\033[0m");
        printf("\033[0;32m请按ESC返回...\033[0m");
        return;
    }
    int cnt = 0;
    cnt = us.grouplist.size();
    int maxpage = cnt / 5, i = 0;
    if(cnt % 5 != 0) maxpage++;
    if(c == '[' && page == 0) ;
    else if(c == '[') page --;
    if(c == ']' && page+1 >= maxpage) ;
    else if(c == ']') page ++;
    printf("\033[0;36m==========================================================\033[0m\n");
    reportfucs::newreport(us, clientp);
    printf("\033[0;32m以下为您加入的群组\033[0m\n");
    printf("\033[0;34m%-6s %-14s %-13s %-12s\033[0m\n", "序号", "群名", "GID", "在群中身份");
    for(std::string str : us.grouplist){
        if(i >= 5*page && i < 5*(page+1)){
            sock->sendMsg("gtgp:"+str);
            std::string red = EchoMsgQueue.wait_and_pop();
            if(red == "norepeat") continue;
            group ud = group::fromJson(red);
            if(ud.owner == "0"){
                printf("\033[0;32m当前群组已注销。\n\033[0m");
                printf("\033[0;32m请按任意键继续...\033[0m");
                charget();
                return;
            }
            std::string name = ud.name, status = "成员";
            const char *color = "\033[0;37m";
            if(ud.owner == us.uid) {
                status = "群主";
                color = "\033[0;31m";
            } else{
                for(std::string str : ud.managelist){
                    if(str == us.uid){
                        status = "管理员";
                        color = "\033[0;32m";
                    }
                }
            }
            printf("\033[0;37m[%d]  %-12s %-14s \033[0m%s%-12s\033[0m",
                     i - 5 * page + 1,
                    name.c_str(), ud.gid.c_str(), color,status.c_str());
            if(rpt.chatgroup[ud.gid]) printf("   \033[0;31m（%d）\033[0m\n", rpt.chatgroup[ud.gid]);
            else puts("");
        }
        i++;
    }
    printf("                     \033[0;32m(tip:按[和]按键可控制翻页)\n\033[0m");
    printf("                                \033[0;32m[%d/%d]\033[0m\n",page+1,maxpage);
    printf("\033[0;36m==========================================================\033[0m\n");
}

void groupfucs::handlequit(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    //找到对应消息
    int i = 5*page + c - '0' - 1, j = 0;
    if(fg == 1)
        if(i >= us.grouplist.size()) return;
    else if(fg == 2)
        if(i >= grouplist.data.size()) return;
    std::string sd;
    if(fg == 1){
        for(std::string str : us.grouplist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    else if(fg == 2){
        for(std::string str : grouplist.data){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    sock->sendMsg("gtgp:"+sd);
    std::string nm = EchoMsgQueue.wait_and_pop(), rev;
    if(nm == "norepeat"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    }
    group gp = group::fromJson(nm);
    nm = gp.name;
    if(gp.owner == us.uid)
        printf("\033[0;32m您为群组\033[0m \033[0;31m%s\033[0m \033[0;32m的群主，退出后该群会解散，确认继续吗？（Y/N）\033[0m\n", nm.c_str());
    else
        printf("\033[0;32m确定要退出群组\033[0m \033[0;31m%s\033[0m \033[0;32m？（Y/N）\033[0m\n", nm.c_str());
    fflush(stdout);
    char input;
    while(1){
        input = charget();
        if(input == 27) return;
        if(input != 'Y' && input != 'N' && input != 'y' && input != 'n') continue;
        if(input == 'Y' || input == 'y') break;
        else return;
        break;
    }
    //确定退出该群聊
    if(gp.owner == us.uid)
        sock->sendMsg("disg:"+gp.gid);
    else
        sock->sendMsg("rmgp:"+us.uid+":"+sd);
    rev = EchoMsgQueue.wait_and_pop();
    if(rev != "right"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        input = charget();
        return ;
    }
    //删除本地列表中的gid
    if(fg == 1)
        us.grouplist.erase(sd);
    else if(fg == 2){
        for(int i = 0; i < grouplist.data.size(); i++)
            if(grouplist.data[i] == sd) grouplist.data.erase(grouplist.data.begin() + i);
    }
}

void groupfucs::quitgroup(){
    system("clear");
    page = 0;
    list('0');
    if(us.grouplist.size())
        printf("\033[0;32m请选择您要退出的群聊:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false;
    std::string msg;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            page = 0;
            us = user::fromJson(msg);
            flag = true;
        }
        //判断是否有新通知
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            list('p');
            if(us.grouplist.size())
                printf("\033[0;32m请选择您要退出的群聊:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        char input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':{
            handlequit(input, 1);
            flag = true;
            break;
        }
        case '[':{
            system("clear");
            list('[');
            if(us.grouplist.size())
                printf("\033[0;32m请选择您要退出的群聊:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            list(']');
            if(us.grouplist.size())
                printf("\033[0;32m请选择您要退出的群聊:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case 27:{
            return ;
        }
        default:continue;
        }
    }
    return ;
}

void groupfucs::viewlist(char c){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if(!fnl.data.size()){
        printf("\033[0;32m没有包含该字段的好友。\n\033[0m");
        printf("\033[0;32m请按ESC返回...\033[0m");
        return;
    }
    int cnt = 0;
    cnt = fnl.data.size();
    int maxpage = cnt / 5, i = 0;
    if(cnt % 5 != 0) maxpage++;
    if(c == '[' && viewpage == 0) ;
    else if(c == '[') viewpage --;
    if(c == ']' && viewpage+1 >= maxpage) ;
    else if(c == ']') viewpage ++;
    printf("\033[0;36m==========================================================\033[0m\n");
    reportfucs::newreport(us, clientp);
    printf("\033[0;32m以下为群聊 %s 的全部成员\033[0m\n", viewgp.name.c_str());
    printf("\033[0;34m%-6s %-15s %-13s %-12s\033[0m\n", "序号", "用户名", "UID", "群中身份");
    for(std::string str : fnl.data){
        if(i >= 5*viewpage && i < 5*(viewpage+1)){
            sock->sendMsg("gtus:"+str);
            std::string red = EchoMsgQueue.wait_and_pop();
            user ud = user::fromJson(red);
            std::string name = ud.name, status, identity = "成员";
            if(ud.stat == "online") status = "在线";
            else if(ud.stat == "offline") status = "离线";
            else if(ud.stat == "destroy") status = "该账户已注销";
            if(ud.uid == viewgp.owner) identity = "群主";
            else if(viewgp.managelist.count(ud.uid)) identity = "管理员";
            // 如果是在线，颜色绿色；否则灰色
            const char *color = (status == "在线") ? "\033[0;32m" : "\033[0;90m";

            printf("%s[%d]  %-12s %-14s %-12s\033[0m",
                    color, i - 5 * viewpage + 1,
                    name.c_str(), ud.uid.c_str(), identity.c_str());
            if(rpt.chatfriend[ud.uid]) printf("   \033[0;31m（%d）\033[0m\n", rpt.chatfriend[ud.uid]);
            else puts("");
        }
        i++;
    }
    printf("                     \033[0;32m(tip:按[和]按键可控制翻页)\n\033[0m");
    printf("                                \033[0;32m[%d/%d]\033[0m\n",viewpage+1,maxpage);
    printf("\033[0;36m==========================================================\033[0m\n");
}

void groupfucs::view(char c, int fg, int hdl){
    Client * cp = (Client*)clientp;
    Socket* sock = cp->getSocket();
    std::string str;
    char arr[512];
    std::string sd;
    system("clear");
    //找到对应群聊
    int i = 5*page + c - '0' - 1, j = 0;
    if(fg == 1)
        if(i >= us.grouplist.size()) return;
    else if(fg == 2)
        if(i >= grouplist.data.size()) return;
    if(fg == 1){
        for(std::string str : us.grouplist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    else if(fg == 2){
        for(std::string str : grouplist.data){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    //获取群成员列表
    sprintf( arr, "gtgp:%s", sd.c_str());
    sock->sendMsg(arr);
    str = EchoMsgQueue.wait_and_pop();
    if(str == "norepeat"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    }
    viewgp = group::fromJson(str);
    fnl.data.clear();
    fnl.data.push_back(viewgp.owner);
    for(std::string tmp : viewgp.managelist) fnl.data.push_back(tmp);
    for(std::string tmp : viewgp.memberlist) fnl.data.push_back(tmp);
    viewpage = 0;
    viewlist('0');
    if(hdl == 1) printf("\033[0;32m请选择您要设置为管理员的成员:>\033[0m");
    else if(hdl == 2) printf("\033[0;32m请选择您要解除管理员的成员:>\033[0m");
    else if(hdl == 3) printf("\033[0;32m请选择您要踢出群聊的成员:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false;
    std::string msg;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            page = 0;
            us = user::fromJson(msg);
            flag = true;
        }
        //判断是否有新通知
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            viewlist('p');
            if(hdl == 1) printf("\033[0;32m请选择您要设置为管理员的成员:>\033[0m");
            else if(hdl == 2) printf("\033[0;32m请选择您要解除管理员的成员:>\033[0m");
            else if(hdl == 3) printf("\033[0;32m请选择您要踢出群聊的成员:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        char input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':{
            if(hdl >= 1 && hdl <= 3){
                int t = 5*viewpage + input - '0' - 1;
                if(t >= fnl.data.size()) continue;
                if(hdl == 1)
                    addmanager( viewgp.gid, fnl.data[t]);
                else if(hdl == 2)
                    delmanager( viewgp.gid, fnl.data[t]);
                else if(hdl == 3)
                    kickmember( viewgp.gid, fnl.data[t]);
                flag = true;
            }
            continue;
        }
        case '[':{
            system("clear");
            viewlist('[');
            if(hdl == 1) printf("\033[0;32m请选择您要设置为管理员的成员:>\033[0m");
            else if(hdl == 2) printf("\033[0;32m请选择您要解除管理员的成员:>\033[0m");
            else if(hdl == 3) printf("\033[0;32m请选择您要踢出群聊的成员:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            viewlist(']');
            if(hdl == 1) printf("\033[0;32m请选择您要设置为管理员的成员:>\033[0m");
            else if(hdl == 2) printf("\033[0;32m请选择您要解除管理员的成员:>\033[0m");
            else if(hdl == 3) printf("\033[0;32m请选择您要踢出群聊的成员:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case 27:{
            return ;
        }
        default:continue;
        }
    }
    return ;
}


void groupfucs::viewmember(){
    system("clear");
    page = 0;
    list('0');
    if(us.grouplist.size())
        printf("\033[0;32m请选择您要查看成员的群聊:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false;
    std::string msg;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            page = 0;
            us = user::fromJson(msg);
            flag = true;
        }
        //判断是否有新通知
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            list('p');
            if(us.grouplist.size())
                printf("\033[0;32m请选择您要查看成员的群聊:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        char input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':{
            view(input, 1);
            flag = true;
            break;
        }
        case '[':{
            system("clear");
            list('[');
            if(us.grouplist.size())
                printf("\033[0;32m请选择您要查看成员的群聊:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            list(']');
            if(us.grouplist.size())
                printf("\033[0;32m请选择您要查看成员的群聊:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case 27:{
            return ;
        }
        default:continue;
        }
    }
    return ;
}

void groupfucs::chatmenu(char c, group& gp){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if(rpt.chatgroup[gp.gid]){
        rpt.total_group_msg -= rpt.chatgroup[gp.gid];
        rpt.chatgroup[gp.gid] = 0;
        sock->sendMsg("svrp:"+us.uid+":"+rpt.toJson());
        std::string rev = EchoMsgQueue.wait_and_pop();
    }
    int cnt = 0;
    cnt = save.data.size();
    int maxctpage = msgcnt / 7;
    int i = 0;
    if(msgcnt % 7 != 0) maxctpage++;
    if(maxctpage == 0) maxctpage = 1;
    if(c == '[' && ctpage == 0) ;
    else if(c == '[') ctpage --;
    if(c == ']' && ctpage+1 >= maxctpage) ;
    else if(c == ']') ctpage ++;
    std::string sender;
    system("clear");
    printf("\033[0;36m=============================群聊页面=============================\033[0m\n");
    reportfucs::newreport(us, clientp);
    printf("\033[0;32m                             %s (%ld)\033[0m\n", gp.name.c_str(), gp.memberlist.size() + gp.managelist.size() + 1);
    if(ctpage+1 == maxctpage)    printf("\033[0;90m                       ---聊天记录已到顶---\033[0m\n");
    for(i = std::min(7*(ctpage+1) - 1, (int)save.data.size() - 1); i >= 7*ctpage; i--){
        if(!save.data.size()) break;
        message msg = message::fromJson(save.data[i]);
        if(msg.sender_uid == us.uid) sender = us.name;
        printf("\033[0;32m%s\033[0m \033[0;33m[%s]\033[0m\n", msg.sender_name.c_str(), msg.timestamp.c_str());
        printf("\033[0;32m>\033[0m%s\n", msg.content.c_str());
    }
    printf("                                     \033[0;32m(tip:按← 和→ 按键可控制翻页，按↑ 发送)\n\033[0m");
    if(enterfuc == 1)
        printf("                                         \033[0;32m(按↓ 可切换回车键功能，当前为换行)\n\033[0m");
    else
        printf("                                         \033[0;32m(按↓ 可切换回车键功能，当前为发送)\n\033[0m");
    printf("                                                         \033[0;32m[%d/%d]\033[0m\n",ctpage+1,maxctpage);
    printf("\033[0;36m==================================================================\033[0m\n");
}

int groupfucs::handlechat(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    printf("\033[0;32m数据请求中...\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    //找到对应gid
    int i = 5*page + c - '0' - 1, j = 0;
    if(fg == 1){
        if(i >= us.grouplist.size()) return 0;
    }
    else if(fg == 2)
        if(i >= grouplist.data.size()) return 0;
    std::string gid;
    if(fg == 1){
        for(std::string str : us.grouplist){
            if(j == i){
                gid = str;
                break;
            }
            j++;
        }
    } 
    else if (fg == 2){
        for(std::string str : grouplist.data){
            if(j == i){
                gid = str;
                break;
            }
            j++;
        }
    }
    sock->sendMsg("gtgp:"+ gid);
    std::string js = EchoMsgQueue.wait_and_pop();
    if(js == "norepeat"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return 0 ;
    }
    viewgp = group::fromJson(js);
    //开始聊天
    //拉取历史记录
    //读取两页消息（一页7句消息）
    save.data.clear();
    sock->sendMsg("gctm:"+gid);
    std::string rev = EchoMsgQueue.wait_and_pop(), msg;
    save = messages::fromJson(rev);
    //读取聊天记录总条数
    sock->sendMsg("rdgc:"+gid);
    rev = EchoMsgQueue.wait_and_pop();
    sscanf(rev.c_str(), "%d", &msgcnt);
    //定义部分变量
    bool flag = false;
    ctpage = 0;
    chatmenu('0', viewgp);
    printf("\033[0;32m请输入:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    std::string content, utf8_buf;
    ChatMsgQueue.clear();
    CtspMsgQueue.clear();
    message sendm;
    startTime();
    enterfuc = 1;
    char cbuf[10];
    while(1){
        int tm = throughtime();
        if(tm >= 1000 || flag){
            //判断用户信息是否变动
            if(UserMsgQueue.try_pop(msg)){
                page = 0;
                us = user::fromJson(msg);
                if(us.grouplist.count(gid) == 0){//被删除了
                    system("clear");
                    printf("\033[0;31m您当前已不在该群聊。\033[0m\n");
                    printf("\033[0;31m请按任意键继续...\033[0m\n");
                    charget();
                    return -1;
                }
                flag = true;
            }
            //判断发完消息的回声
            if(CtspMsgQueue.try_pop(msg)){
                bool tmout = false, nofrd = false, disgp = false;
                if(msg == "time_out") tmout = true;
                else if(msg == "nofrd") nofrd = true;
                else if(msg == "disgp") disgp = true;
                while(CtspMsgQueue.try_pop(msg)){
                    if(msg == "time_out") tmout = true;
                    else if(msg == "nofrd") nofrd = true;
                }
                if(tmout){
                    system("clear");
                    printf("\033[0;31m发送超时！\033[0m\n");
                    printf("\033[0;31m请按任意键继续...\033[0m\n");
                    charget();
                    CtspMsgQueue.clear();
                    return 0;
                }
                else if(nofrd){
                    system("clear");
                    printf("\033[0;31m当前不在群内，发送失败！\033[0m\n");
                    printf("\033[0;31m请按任意键继续...\033[0m\n");
                    charget();
                    CtspMsgQueue.clear();
                    return -1;
                }
                else if(disgp){
                    system("clear");
                    printf("\033[0;31m该群聊已解散，发送失败！\033[0m\n");
                    printf("\033[0;31m请按任意键继续...\033[0m\n");
                    charget();
                    CtspMsgQueue.clear();
                    return -1;
                }
            }
            //判断聊天消息是否有新
            while(ChatMsgQueue.try_pop(msg)){
                page = 0;
                message m = message::fromJson(msg);
                if(m.sender_uid != us.uid && m.receiver_uid == gid && m.is_group) save.data.insert(save.data.begin(), msg);
                msgcnt++;
                flag = true;
            }
            //判断是否有新通知
            while(ReptMsgQueue.try_pop(msg)){
                if(msg == "disg" || msg == "modmanage") continue;
                rpt = report::fromJson(msg);
                flag = true;
            }
            if(flag){
                flag = false;
                chatmenu('p', viewgp);
                printf("\033[0;32m请输入:>\033[0m");
                printf("%s", content.c_str());
                fflush(stdout); // 手动刷新标准输出缓冲区
            }
            startTime();
        }
        int input = sptm_charget(1000, cbuf);
        if(input == -1) continue;
        // 中文或其他 UTF-8 字符处理
        switch(input){
        case KEY_ESC:{
            CtspMsgQueue.clear();
            return 0;
        }
        case KEY_UP:{
            if(content.size() == 0 || content == "\n") break;
            content.push_back('\0');
            sendm.sender_uid = us.uid;
            sendm.sender_name = us.name;
            sendm.receiver_uid = gid;
            sendm.content = content;
            sendm.timestamp = message::get_beijing_time();
            sendm.is_group = true;
            sock->sendMsg("sdgm:"+sendm.toJson());
            save.data.insert(save.data.begin(), sendm.toJson());
            flag = true;
            ctpage = 0;
            content.clear();
            msgcnt++;
            break;
        }
        case KEY_DOWN:{
            enterfuc = 1 - enterfuc;
            flag = true;
            break;
        }
        case KEY_LEFT:{
            chatmenu('[', viewgp);
            printf("\033[0;32m请输入:>\033[0m");
            printf("%s", content.c_str());
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case KEY_RIGHT:{
            //请求新页的消息
            if(save.data.size() < msgcnt){
                char tmp[512];
                sprintf(tmp, "ndgm:%s:%ld", gid.c_str(), save.data.size());
                sock->sendMsg(tmp);
                messages mgs = messages::fromJson(EchoMsgQueue.wait_and_pop());
                for(int i = 0; i < mgs.data.size(); i++)
                    save.data.push_back(mgs.data[i]);
            }
            chatmenu(']', viewgp);
            printf("\033[0;32m请输入:>\033[0m");
            printf("%s", content.c_str());
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        default:{
            if(enterfuc == 0 &&  (std::string(cbuf) == "\n" || std::string(cbuf) == "\r" )){
                if(content.size() == 0 || content == "\n") break;
                content.push_back('\0');
                sendm.sender_uid = us.uid;
                sendm.sender_name = us.name;
                sendm.receiver_uid = gid;
                sendm.content = content;
                sendm.timestamp = message::get_beijing_time();
                sendm.is_group = true;
                sock->sendMsg("sdgm:"+sendm.toJson());
                save.data.insert(save.data.begin(), sendm.toJson());
                flag = true;
                ctpage = 0;
                content.clear();
                msgcnt++;
                continue;
            }
            if ((unsigned char)cbuf[0] == 8 || (unsigned char)cbuf[0] == 127) {
                if (content.empty()) continue;
                int i = content.size() - 1;
                int len = 1;
                // 找到 UTF-8 字符的起始位置（跳过多字节的 continuation bytes）
                while (i - len >= 0 && (static_cast<unsigned char>(content[i - len + 1]) & 0xC0) == 0x80) {
                    len++;
                }
                int char_start = i - len + 1;
                if (char_start < 0 || char_start >= (int)content.size()) continue;
                std::string ch = content.substr(char_start, len);
                // 判断字符显示宽度（中文2宽度，英文1宽度）
                int display_width = is_wide_char((const unsigned char *)ch.c_str()) ? 2 : 1;
                // 从内容中删除该字符
                content.erase(char_start, len);
                // 从终端回退光标
                for (int j = 0; j < display_width; ++j) {
                    printf("\b \b");
                }
                fflush(stdout);
                continue;
            }
            if(content.size() >= 1000) continue;
            // 拼完字符
            content += std::string(cbuf);
            printf("%s", cbuf);
            fflush(stdout);
            memset(cbuf, 0, sizeof cbuf);
            break;
        }
        }
    }
}

void groupfucs::groupchat(){
    system("clear");
    page = 0;
    list('0');
    if(us.grouplist.size())
        printf("\033[0;32m请选择您要开始聊天的群:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false;
    std::string msg;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            page = 0;
            us = user::fromJson(msg);
            flag = true;
        }
        //判断是否有新通知
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            list('p');
            if(us.grouplist.size())
                printf("\033[0;32m请选择您要开始聊天的群:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        char input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':{
            handlechat(input, 1);
            flag = true;
            break;
        }
        case '[':{
            system("clear");
            list('[');
            if(us.grouplist.size())
                printf("\033[0;32m请选择您要开始聊天的群:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            list(']');
            if(us.grouplist.size())
                printf("\033[0;32m请选择您要开始聊天的群:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case 27:{
            return ;
        }
        default:continue;
        }
    }
    return ;
}

void groupfucs::list_group(){
    system("clear");
    page = 0;
    list('0');
    if(us.grouplist.size())
        printf("\033[0;32m输入序号可进行操作:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false;
    std::string msg;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            page = 0;
            us = user::fromJson(msg);
            flag = true;
        }
        //判断是否有新通知
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            system("clear");
            list('p');
            if(us.grouplist.size())
                printf("\033[0;32m输入序号可进行操作:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        char input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':{
            int i = 5*page + input - '0' - 1;
            if(i >= 0 && i < us.grouplist.size()){
                select(input, 1);
                flag = true;
            }
            break;
        }
        case '[':{
            system("clear");
            list('[');
            if(us.grouplist.size())
                printf("\033[0;32m输入序号可进行操作:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            list(']');
            if(us.grouplist.size())
                printf("\033[0;32m输入序号可进行操作:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case 27:{
            return ;
        }
        default:continue;
        }
    }
    return ;
}

int groupfucs::selectmenu(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    //找到对应消息
    int i = 5*page + c - '0' - 1, j = 0;
    if(fg == 1)
        if(i >= us.grouplist.size()) return 0;
    else if(fg == 2)
        if(i >= grouplist.data.size()) return 0;
    std::string sd;
    if(fg == 1){
        for(std::string str : us.grouplist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    } else if (fg == 2){
        for(std::string str : grouplist.data){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    sock->sendMsg("gtgp:"+sd);
    std::string rev = EchoMsgQueue.wait_and_pop();
    if(rev == "norepeat"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n]]");
        printf("\033[0;31m请按任意键继续...\033[0m\n");
        charget();
        return -1;
    }
    group gp = group::fromJson(rev);
    system("clear");
    fflush(stdout); // 手动刷新标准输出缓冲区
    printf("\033[0;32m数据请求中...\033[0m\n");
    fflush(stdout); // 手动刷新标准输出缓冲区
    system("clear");
    identity = "成员";
    if(gp.owner == us.uid) identity = "群主";
    else if(gp.managelist.count(us.uid)) identity = "管理员";
    printf("\033[0;36m==========================================================\033[0m\n");
    printf("                 \033[0;33m您是群聊 %s 的 %s\033[0m\n", gp.name.c_str(), identity.c_str());
    reportfucs::newreport(us, clientp);
    printf("\033[0;32m[1] 开始群聊\033[0m\n");
    if(identity == "群主")
        printf("\033[0;32m[2] 解散群聊\033[0m\n");
    else
        printf("\033[0;32m[2] 退出群聊\033[0m\n");
    printf("\033[0;32m[3] 查看群成员\033[0m\n");
    if(identity != "成员"){
        printf("\033[0;32m[4] 踢出群成员\033[0m\n");
    }
    if(identity == "群主"){
        printf("\033[0;32m[5] 添加管理员\033[0m\n");
        printf("\033[0;32m[6] 删除管理员\033[0m\n");
    }
    printf("                    \033[0;33m(tip:ESC可返回上一级菜单)\033[0m\n");
    printf("\033[0;36m==========================================================\033[0m\n");
    printf("\033[0;32m输入序号可进行操作:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    return 0;
}

//fg == 1为正常选中，fg == 2则为搜索选中
void groupfucs::select(char c, int fg){
    system("clear");
    selectmenu(c, fg);
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false;
    std::string msg;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            us = user::fromJson(msg);
            flag = true;
        }
        //判断是否有新通知
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "modmanage") continue;
            flag = true;
            if(msg == "disg") break;
            rpt = report::fromJson(msg);
        }
        if(flag){
            flag = false;
            system("clear");
            if(msg == "disg") return;
            selectmenu(c, fg);
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        char input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':{
            if(handlechat(c, fg) == -1) return;
            flag = true;
            break;
        }
        case '2':{
            if(identity == "群主"){
                disband(c, fg);
            }
            else handlequit(c, fg);
            flag = true;
            return;
        }
        case '3':{
            view(c, fg);
            flag = true;
            break;
        }
        case '4':{
            //踢人
            if(identity != "成员"){
                view(c, fg, 3);
                flag = true;
            }
            break;
        }
        case '5':{
            //设置管理员
            if(identity == "群主"){
                view(c, fg, 1);
                flag = true;
            }
            break;
        }
        case '6':{
            //删除管理员
            if(identity == "群主"){
                view(c, fg, 2);
                flag = true;
            }
            break;
        }
        case 27:{
            return ;
        }
        default:continue;
        }
    }
    return ;
}

void groupfucs::disband(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    std::string sd, str;
    int i = 5*page + c - '0' - 1, j = 0;
    if(fg == 1)
        if(i >= us.grouplist.size()) return;
    else if(fg == 2)
        if(i >= grouplist.data.size()) return;
    if(fg == 1){
        for(std::string str : us.grouplist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    else if(fg == 2){
        for(std::string str :grouplist.data){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    //获取群成员列表
    char arr[512];
    sprintf( arr, "gtgp:%s", sd.c_str());
    sock->sendMsg(arr);
    str = EchoMsgQueue.wait_and_pop();
    if(str == "norepeat"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    }
    viewgp = group::fromJson(str);

    system("clear");
    printf("\033[0;31m确定要解散群聊 %s ？（Y/N）\033[0m\n", viewgp.name.c_str());
    fflush(stdout);
    char input;
    bool off = false;
    while(1){
        input = charget();
        if(input == 27) break;
        if(input != 'Y' && input != 'N' && input != 'y' && input != 'n') continue;
        if(input == 'Y' || input == 'y') off = true;
        break;
    }
    if(!off) return ;
    off = false;   
    printf("\033[0;31m真的确定要解散群聊 %s ？（T/F）\033[0m\n", viewgp.name.c_str());
    while(1){
        input = charget();
        if(input == 27) break;
        if(input != 'T' && input != 't' && input != 'F' && input != 'f') continue;
        if(input == 'T' || input == 't') off = true;
        break;
    }
    if(!off) return ;

    sock->sendMsg("disg:"+viewgp.gid);
    str = EchoMsgQueue.wait_and_pop();
    if(str != "right"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    }
    printf("\033[0;32m已解散该群聊\033[0m\n");
    printf("\033[0;32m请按任意键继续...\033[0m");
    charget();
    return ;
}


void groupfucs::delmanager(std::string gid, std::string uid2) {
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    if(uid2 == us.uid || viewgp.memberlist.count(uid2)){
        printf("\033[0;31m该用户不为管理员，无法操作。\n\033[0m");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return;
    }
    //发uid，获取是管理员的群聊列表
    sock->sendMsg("gtnm:"+uid2);
    std::string nm = EchoMsgQueue.wait_and_pop(), rev;
    printf("\033[0;32m确定将成员\033[0m \033[0;31m%s\033[0m \033[0;32m解除管理员？（Y/N）\033[0m\n", nm.c_str());
    fflush(stdout);
    char input;
    while(1){
        input = charget();
        if(input == 27) return;
        if(input != 'Y' && input != 'N' && input != 'y' && input != 'n') continue;
        if(input == 'Y' || input == 'y') break;
        else return;
        break;
    }
    //确定设置uid2为管理员
    sock->sendMsg("dlmn:"+gid+":"+ uid2);
    rev = EchoMsgQueue.wait_and_pop();
    if(rev != "right"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    }
    //改本地表
    viewgp.managelist.erase(uid2);
    viewgp.memberlist.insert(uid2);
    system("clear");
    printf("\033[0;32m解除管理员成功。\033[0m\n");
    printf("\033[0;32m请按任意键继续...\033[0m");
    fflush(stdout);
    charget();
}

void groupfucs::kickmember(std::string gid, std::string uid2) {
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    if(!(viewgp.managelist.count(uid2) && us.uid == viewgp.owner)){
        if(uid2 == viewgp.owner || viewgp.managelist.count(uid2)){
            if(uid2 == us.uid)
                printf("\033[0;31m你无法将自己踢出群聊。\n\033[0m");
            else if(uid2 == viewgp.owner)
                printf("\033[0;31m你无法将群主踢出群聊。\n\033[0m");    
            else if(viewgp.managelist.count(uid2)) 
                printf("\033[0;31m你无法将其他管理员踢出群聊。\n\033[0m");
            printf("\033[0;31m请按任意键继续...\033[0m");
            charget();
            return;
        }
    }
    //发uid，获取是管理员的群聊列表
    sock->sendMsg("gtnm:"+uid2);
    std::string nm = EchoMsgQueue.wait_and_pop(), rev;
    printf("\033[0;32m确定踢出成员\033[0m \033[0;31m%s\033[0m \033[0;32m？（Y/N）\033[0m\n", nm.c_str());
    fflush(stdout);
    char input;
    while(1){
        input = charget();
        if(input == 27) return;
        if(input != 'Y' && input != 'N' && input != 'y' && input != 'n') continue;
        if(input == 'Y' || input == 'y') break;
        else return;
        break;
    }
    //确定设置uid2为管理员
    sock->sendMsg("kcmb:"+gid+":"+ uid2+":"+us.uid);
    rev = EchoMsgQueue.wait_and_pop();
    if(rev == "false"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    } else if(rev == "nopms"){
        printf("\033[0;31m当前不是管理员，权限不足。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    } else if(rev == "pmser"){
        printf("\033[0;31m你无法将其他管理员踢出群聊。\n\033[0m");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return;
    }
    //改本地表
    viewgp.memberlist.erase(uid2);
    viewgp.managelist.erase(uid2);
    for(int i = 0; i < fnl.data.size(); i++){
        if(fnl.data[i] == uid2){
            fnl.data.erase(fnl.data.begin() + i);
            break;
        }
    }
    system("clear");
    printf("\033[0;32m踢出群成员成功。\033[0m\n");
    printf("\033[0;32m请按任意键继续...\033[0m");
    fflush(stdout);
    input = charget();
}

//当前用户设置uid2为管理员
void groupfucs::addmanager(std::string gid, std::string uid2){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    if(uid2 == us.uid){
        printf("\033[0;31m无法设置自己为管理员。\n\033[0m");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return;
    } else if(viewgp.managelist.count(uid2)){
        printf("\033[0;31m该成员已是管理员。\n\033[0m");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return;
    }
    //发uid，获取是管理员的群聊列表
    sock->sendMsg("gtnm:"+uid2);
    std::string nm = EchoMsgQueue.wait_and_pop(), rev;
    printf("\033[0;32m确定将成员\033[0m \033[0;31m%s\033[0m \033[0;32m设置为管理员？（Y/N）\033[0m\n", nm.c_str());
    fflush(stdout);
    char input;
    while(1){
        input = charget();
        if(input == 27) return;
        if(input != 'Y' && input != 'N' && input != 'y' && input != 'n') continue;
        if(input == 'Y' || input == 'y') break;
        else return;
        break;
    }
    //确定设置uid2为管理员
    sock->sendMsg("admn:"+gid+":"+ uid2);
    rev = EchoMsgQueue.wait_and_pop();
    if(rev != "right"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        input = charget();
        return ;
    }
    //改本地表
    viewgp.memberlist.erase(uid2);
    viewgp.managelist.insert(uid2);
    system("clear");
    printf("\033[0;32m添加管理员成功。\033[0m\n");
    printf("\033[0;32m请按任意键继续...\033[0m");
    fflush(stdout);
    input = charget();
}

void groupfucs::managelist(char c){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if(!grouplist.data.size()){
        printf("\033[0;32m您当前不是任何群的管理员。\n\033[0m");
        printf("\033[0;32m请按ESC返回...\033[0m");
        return;
    }
    int cnt = 0;
    cnt = grouplist.data.size();
    int maxpage = cnt / 5, i = 0;
    if(cnt % 5 != 0) maxpage++;
    if(c == '[' && page == 0) ;
    else if(c == '[') page --;
    if(c == ']' && page+1 >= maxpage) ;
    else if(c == ']') page ++;
    printf("\033[0;36m==========================================================\033[0m\n");
    reportfucs::newreport(us, clientp);
    printf("\033[0;32m以下为您有权限的群聊\033[0m\n");
    printf("\033[0;34m%-6s %-15s %-13s %-12s\033[0m\n", "序号", "群名", "GID", "在群中身份");
    for(std::string str : grouplist.data){
        if(i >= 5*page && i < 5*(page+1)){
            sock->sendMsg("gtgp:"+str);
            std::string red = EchoMsgQueue.wait_and_pop();
            if(red == "norepeat") continue;
            group ud = group::fromJson(red);
            if(ud.owner == "0"){
                printf("\033[0;32m当前群组已注销。\n\033[0m");
                continue;
            }
            std::string name = ud.name, status = "成员";
            const char *color = "\033[0;37m";
            if(ud.owner == us.uid) {
                status = "群主";
                color = "\033[0;31m";
            } else{
                for(std::string str : ud.managelist){
                    if(str == us.uid){
                        status = "管理员";
                        color = "\033[0;32m";
                    }
                }
            }
            printf("\033[0;37m[%d]  %-12s %-14s \033[0m%s%-12s\033[0m",
                     i - 5 * page + 1,
                    name.c_str(), ud.gid.c_str(), color,status.c_str());
            if(rpt.chatgroup[ud.gid]) printf("   \033[0;31m（%d）\033[0m\n", rpt.chatgroup[ud.gid]);
            else puts("");
        }
        i++;
    }
    printf("                     \033[0;32m(tip:按[和]按键可控制翻页)\n\033[0m");
    printf("                                \033[0;32m[%d/%d]\033[0m\n",page+1,maxpage);
    printf("\033[0;36m==========================================================\033[0m\n");
}

void groupfucs::manage(){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    //发uid，获取是管理员的群聊列表
    sock->sendMsg("mngl:" + us.uid);
    std::string rev = EchoMsgQueue.wait_and_pop();
    grouplist = friendnamelist::fromJson(rev);
    if(!grouplist.data.size()){
        printf("\033[0;32m您当前不是任何群的管理员。\n\033[0m");
        printf("\033[0;32m请按任意键继续...\033[0m");
        charget();
        return;
    }

    page = 0;
    managelist('0');
    if(grouplist.data.size())
        printf("\033[0;32m输入序号可进行操作:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false;
    std::string msg;
    while(1){
        //判断用户信息是否变动
        if(UserMsgQueue.try_pop(msg)){
            page = 0;
            us = user::fromJson(msg);
            flag = true;
        }
        //判断是否有新通知
        while(ReptMsgQueue.try_pop(msg)){
            if(msg == "disg") continue;
            flag = true;
            if(msg == "modmanage") break;
            rpt = report::fromJson(msg);
        }
        if(flag){
            flag = false;
            system("clear");
            if(msg == "modmanage"){
                sock->sendMsg("mngl:" + us.uid);
                rev = EchoMsgQueue.wait_and_pop();
                grouplist = friendnamelist::fromJson(rev);
            }
            managelist('p');
            if(grouplist.data.size())
                printf("\033[0;32m输入序号可进行操作:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        char input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':{
            int p = 5*page + input - '0' - 1;
            if(p >= 0 && p < grouplist.data.size()){
                select(input, 2);
                flag = true;
            }
            break;
        }
        case '[':{
            system("clear");
            managelist('[');
            if(grouplist.data.size())
                printf("\033[0;32m输入序号可进行操作:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            managelist(']');
            if(grouplist.data.size())
                printf("\033[0;32m输入序号可进行操作:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case 27:{
            return ;
        }
        default:continue;
        }
    }
    return;
}

