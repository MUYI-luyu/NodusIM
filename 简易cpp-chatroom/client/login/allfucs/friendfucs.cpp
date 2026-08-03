#include "./friendfucs.h"

void friendfucs::addfriend(){
    Client* c = (Client*)clientp;
    Socket* sock = c->getSocket();
    system("clear");
    printf("\033[0;32m已选择添加好友功能(按ESC可返回上一级菜单)\033[0m\n");
    std::string red;
    char buf[520];
    char uid[256];
    printf("\033[0;32m请输入您要添加好友的uid:\n\033[0m>");
    while(1){
        chu(uid);
        if(enter(uid, 0) == -1) return ; 
        if(strlen(uid) <= 3){
            printf("\033[0;31m该uid无效，请重新输入。\033[0m\n>");
            continue;
        }
        if(uid == us.uid){
            printf("\033[0;31m不能添加自己为好友，请重新输入。\033[0m\n>");
            continue;
        }
        //判断此人是否存在
        sprintf(buf, "jrud:%s", uid);
        sock->sendMsg(buf);
        red = EchoMsgQueue.wait_and_pop();
        if(red == "norepeat"){
            printf("\033[0;31m该uid无效，请重新输入。\033[0m\n>");
            continue;
        }
        //判断是否已为好友
        chu(buf);
        sprintf(buf, "jrfd:%s:%s", uid, us.uid.c_str());
        sock->sendMsg(buf);
        red = EchoMsgQueue.wait_and_pop();
        if(red == "right"){
            printf("\033[0;31m您与%s用户已为好友，无需添加，请重新输入。\033[0m\n>", uid);
            continue;
        }
        
        chu(buf);
        red.clear();
        printf("\033[0;32m正在发送好友申请...\n\033[0m");
        fflush(stdout); // 手动刷新标准输出缓冲区
        //给服务器处理
        chu(buf);
        sprintf(buf, "adfr:%s:%s",us.uid.c_str(), uid);
        sock->sendMsg(buf);
        red = EchoMsgQueue.wait_and_pop();
        if(red == "right") {
            printf("\033[0;32m好友申请发送成功，请按任意键继续...\n\033[0m");
            char rub;
            rub = charget();
            return;
        } else if(red == "sendd"){
            printf("\033[0;31m已对该用户发送过好友申请，请等待对方验证。\033[0m\n>");
            continue;
        }
        else if(red == "dstry"){
            printf("\033[0;31m该用户已注销，无法发送好友申请。\033[0m\n>");
            continue;
        }
        else{
            printf("\033[0;31m数据异常，请稍后再试。\033[0m\n>");
            continue;
        }
    }
    return ;

}

void friendfucs::list(char c){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if(!us.friendlist.size()){
        system("clear");
        printf("\033[0;32m当前没有好友。\n\033[0m");
        printf("\033[0;32m请按ESC返回...\033[0m");
        return;
    }
    int cnt = 0;
    cnt = us.friendlist.size();
    int maxpage = cnt / 5, i = 0;
    if(cnt % 5 != 0) maxpage++;
    if(c == '[' && page == 0) ;
    else if(c == '[') page --;
    if(c == ']' && page+1 >= maxpage) ;
    else if(c == ']') page ++;
    system("clear");
    printf("\033[0;36m==========================================================\033[0m\n");
    reportfucs::newreport(us, clientp);
    printf("\033[0;32m以下为您的好友\033[0m\n");
    printf("\033[0;34m%-6s %-15s %-13s %-12s\033[0m\n", "序号", "用户名", "UID", "在线状态");
    for(std::string str : us.friendlist){
        if(i >= 5*page && i < 5*(page+1)){
            sock->sendMsg("gtus:"+str);
            std::string red = EchoMsgQueue.wait_and_pop();
            user ud = user::fromJson(red);
            std::string name = ud.name, status;
            if(ud.stat == "online") status = "在线";
            else if(ud.stat == "offline") status = "离线";
            else if(ud.stat == "destroy") status = "该账户已注销";

            // 如果是在线，颜色绿色；否则灰色
            const char *color = (status == "在线") ? "\033[0;32m" : "\033[0;90m";

            printf("%s[%d]  %-12s %-14s %-12s\033[0m",
                    color, i - 5 * page + 1,
                    name.c_str(), ud.uid.c_str(), status.c_str());
            if(rpt.chatfriend[ud.uid]) printf("   \033[0;31m（%d）\033[0m\n", rpt.chatfriend[ud.uid]);
            else puts("");
        }
        i++;
    }
    printf("                     \033[0;32m(tip:按[和]按键可控制翻页)\n\033[0m");
    printf("                                \033[0;32m[%d/%d]\033[0m\n",page+1,maxpage);
    printf("\033[0;36m==========================================================\033[0m\n");
}

void friendfucs::selectmenu(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    //找到对应消息
    int i = 5*page + c - '0' - 1, j = 0;
    if(fg == 1)
        if(i >= us.friendlist.size()) return;
    else if(fg == 2)
        if(i >= fnl.data.size()) return;
    std::string sd;
    if(fg == 1){
        for(std::string str : us.friendlist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    } else if (fg == 2){
        for(std::string str : fnl.data){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    sock->sendMsg("gtnm:"+sd);
    std::string nm = EchoMsgQueue.wait_and_pop(), rev;
    system("clear");
    fflush(stdout); // 手动刷新标准输出缓冲区
    printf("\033[0;32m数据请求中...\033[0m\n");
    fflush(stdout); // 手动刷新标准输出缓冲区
    system("clear");
    printf("\033[0;36m==========================================================\033[0m\n");
    printf("               \033[0;33m请选择对好友 %s 要进行的操作\033[0m\n", nm.c_str());
    reportfucs::newreport(us, clientp);
    printf("\033[0;32m[1] 删除好友\033[0m\n");
    printf("\033[0;32m[2] 私聊好友\033[0m\n");
    printf("\033[0;32m[3] 屏蔽好友\033[0m\n");
    printf("\033[0;32m[4] 解除屏蔽好友\033[0m\n");
    printf("                    \033[0;33m(tip:ESC可返回上一级菜单)\033[0m\n");
    printf("\033[0;36m==========================================================\033[0m\n");
    printf("\033[0;32m输入序号可进行操作:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
}

//fg == 1为正常选中，fg == 2则为搜索选中
void friendfucs::select(char c, int fg){
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
            if(msg == "disg" || msg == "modmanage") continue;
            rpt = report::fromJson(msg);
            flag = true;
        }
        if(flag){
            flag = false;
            selectmenu(c, fg);
            fflush(stdout); // 手动刷新标准输出缓冲区
        }
        char input = tm_charget(1000);
        if(input == -1) continue;
        switch(input){
        case '1':{
            handledel(c, fg);
            flag = true;
            break;
        }
        case '2':{
            handlechat(c, fg);
            flag = true;
            break;
        }
        case '3':{
            shield(c, fg);
            flag = true;
            break;
        }
        case '4':{
            shield_exit(c, fg);
            flag = true;
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

void friendfucs::listfriend(){
    page = 0;
    list('0');
    if(us.friendlist.size())
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
            list('p');
            if(us.friendlist.size())
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
            if(p >= 0 && p < us.friendlist.size()){
                select(input, 1);
                flag = true;
            }
            break;
        }
        case '[':{
            list('[');
            if(us.friendlist.size())
                printf("\033[0;32m输入序号可进行操作:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            list(']');
            if(us.friendlist.size())
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

void friendfucs::handledel(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    //找到对应消息
    int i = 5*page + c - '0' - 1, j = 0;
    if(fg == 1)
        if(i >= us.friendlist.size()) return;
    else if(fg == 2)
        if(i >= fnl.data.size()) return;
    std::string sd;
    if(fg == 1){
        for(std::string str : us.friendlist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    } else if(fg == 2){
        for(std::string str : fnl.data){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    sock->sendMsg("gtnm:"+sd);
    std::string nm = EchoMsgQueue.wait_and_pop(), rev;
    printf("\033[0;32m确定要删除好友\033[0m \033[0;31m%s\033[0m \033[0;32m？（Y/N）\033[0m\n", nm.c_str());
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
    //确定删除该好友,uid1删uid2
    sock->sendMsg("rmfd:"+us.uid+":"+sd);
    rev = EchoMsgQueue.wait_and_pop();
    if(rev != "right"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        input = charget();
        return ;
    }
    //删除本地列表中的uid2
    us.friendlist.erase(sd);
    for(int i = 0; i < fnl.data.size(); i++)
    if(fnl.data[i] == sd) fnl.data.erase(fnl.data.begin() + i);
}


void friendfucs::delfriend(){
    page = 0;
    list('0');
    if(us.friendlist.size())
        printf("\033[0;32m请选择您要删除的好友:>\033[0m");
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
            list('p');
            if(us.friendlist.size())
                printf("\033[0;32m请选择您要删除的好友:>\033[0m");
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
            handledel(input);
            flag = true;
            break;
        }
        case '[':{
            list('[');
            if(us.friendlist.size())
                printf("\033[0;32m请选择您要删除的好友:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            list(']');
            if(us.friendlist.size())
                printf("\033[0;32m请选择您要删除的好友:>\033[0m");
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

void friendfucs::chatmenu(char c, user& ud2){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if( rpt.chatfriend[ud2.uid]){
        rpt.total_friend_msg -= rpt.chatfriend[ud2.uid];
        rpt.chatfriend[ud2.uid] = 0;
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
    printf("\033[0;36m=============================聊天页面=============================\033[0m\n");
    reportfucs::newreport(us, clientp);
    if(ud2.stat == "online") printf("\033[0;32m                             %s (在线)\033[0m\n", ud2.name.c_str());
    else                     printf("\033[0;90m                             %s (离线)\033[0m\n", ud2.name.c_str());
    if(ctpage+1 == maxctpage)    printf("\033[0;90m                       ---聊天记录已到顶---\033[0m\n");
    for(i = std::min(7*(ctpage+1) - 1, (int)save.data.size() - 1); i >= 7*ctpage; i--){
        if(!save.data.size()) break;
        message msg = message::fromJson(save.data[i]);
        sender = ud2.name;
        if(msg.sender_uid == us.uid) sender = us.name;
        if(ud2.stat == "online" || sender == us.name) printf("\033[0;32m%s\033[0m \033[0;33m[%s]\033[0m\n", sender.c_str(), msg.timestamp.c_str());
        else                     printf("\033[0;90m%s\033[0m \033[0;33m[%s]\033[0m\n", sender.c_str(), msg.timestamp.c_str());
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

void friendfucs::handlechat(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    printf("\033[0;32m数据请求中...\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    //找到对应uid
    int i = 5*page + c - '0' - 1, j = 0;
    if(fg == 1){
        if(i >= us.friendlist.size()) return;
    }
    else if(fg == 2)
        if(i >= fnl.data.size()) return;
    std::string uid2;
    if(fg == 1){
        for(std::string str : us.friendlist){
            if(j == i){
                uid2 = str;
                break;
            }
            j++;
        }
    } else if (fg == 2){
        for(std::string str : fnl.data){
            if(j == i){
                uid2 = str;
                break;
            }
            j++;
        }
    }
    sock->sendMsg("gtus:"+ uid2);
    std::string js2 = EchoMsgQueue.wait_and_pop();
    user ud2 = user::fromJson(js2);
    //开始聊天
    //拉取历史记录
    //读取两页消息（一页7句消息）
    sock->sendMsg("ctms:"+us.uid+":"+uid2);
    std::string rev = EchoMsgQueue.wait_and_pop(), msg;
    save = messages::fromJson(rev);
    //读取聊天记录总条数
    sock->sendMsg("rdpg:"+ us.uid + ":" + uid2);
    rev = EchoMsgQueue.wait_and_pop();
    sscanf(rev.c_str(), "%d", &msgcnt);
    //定义部分变量
    bool flag = false;
    system("clear");
    ctpage = 0;
    chatmenu('0', ud2);
    printf("\033[0;32m请输入:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    std::string content, utf8_buf;
    ChatMsgQueue.clear();
    CtspMsgQueue.clear();
    message sendm;
    startTime();
    char cbuf[10];
    enterfuc = 1;
    while(1){
        int tm = throughtime();
        if(tm >= 1000 || flag){
            //判断用户信息是否变动
            if(UserMsgQueue.try_pop(msg)){
                page = 0;
                us = user::fromJson(msg);
                if(us.friendlist.count(uid2) == 0){//被删除了
                    system("clear");
                    printf("\033[0;31m您被对方删除了。\033[0m\n");
                    printf("\033[0;31m请按任意键继续...\033[0m\n");
                    charget();
                    return;
                }
                flag = true;
            }
            //判断发完消息的回声
            if(CtspMsgQueue.try_pop(msg)){
                bool tmout = false, nofrd = false;
                int sheld = 0;
                if(msg == "time_out") tmout = true;
                else if(msg == "nofrd") nofrd = true;
                else if(msg == "sheld") sheld ++;
                while(CtspMsgQueue.try_pop(msg)){
                    if(msg == "time_out") tmout = true;
                    else if(msg == "nofrd") nofrd = true;
                    else if(msg == "sheld") sheld ++;
                }
                if(sheld <= save.data.size())
                    save.data.erase(save.data.begin(), save.data.begin() + sheld);
                else save.data.clear();
                if(tmout){
                    system("clear");
                    printf("\033[0;31m发送超时！\033[0m\n");
                    printf("\033[0;31m请按任意键继续...\033[0m\n");
                    charget();
                    CtspMsgQueue.clear();
                    return;
                }
                else if(nofrd){
                    system("clear");
                    printf("\033[0;31m当前不是好友，发送失败！\033[0m\n");
                    printf("\033[0;31m请按任意键继续...\033[0m\n");
                    charget();
                    CtspMsgQueue.clear();
                    return;
                } else if(sheld){
                    system("clear");
                    printf("\033[0;31m对方已将你拉黑，消息被拒收！\033[0m\n");
                    printf("\033[0;31m请按任意键继续...\033[0m\n");
                    charget();
                    flag = true;
                    continue;
                }
            }
            //判断聊天消息是否有新
            while(ChatMsgQueue.try_pop(msg)){
                page = 0;
                message m = message::fromJson(msg);
                if(m.sender_uid == ud2.uid && m.is_group == false) save.data.insert(save.data.begin(), msg);
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
                chatmenu('p', ud2);
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
            return ;
        }
        case KEY_UP:{
            if(content.size() == 0 || content == "\n") break;
            content.push_back('\0');
            sendm.sender_uid = us.uid;
            sendm.sender_name = us.name;
            sendm.receiver_uid = uid2;
            sendm.content = content;
            sendm.timestamp = message::get_beijing_time();
            sock->sendMsg("sdms:"+sendm.toJson());
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
            chatmenu('[', ud2);
            printf("\033[0;32m请输入:>\033[0m");
            printf("%s", content.c_str());
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case KEY_RIGHT:{
            //请求新页的消息
            if(save.data.size() < msgcnt){
                char tmp[512];
                sprintf(tmp, "ndms:%s:%s:%ld", us.uid.c_str(), ud2.uid.c_str(), save.data.size());
                sock->sendMsg(tmp);
                messages mgs = messages::fromJson(EchoMsgQueue.wait_and_pop());
                for(int i = 0; i < mgs.data.size(); i++)
                    save.data.push_back(mgs.data[i]);
            }
            chatmenu(']', ud2);
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
                sendm.receiver_uid = uid2;
                sendm.content = content;
                sendm.timestamp = message::get_beijing_time();
                sock->sendMsg("sdms:"+sendm.toJson());
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

void friendfucs::chatfriend(){
    page = 0;
    list('0');
    if(us.friendlist.size())
        printf("\033[0;32m请选择您要私聊的好友:>\033[0m");
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
            list('p');
            if(us.friendlist.size())
                printf("\033[0;32m请选择您要私聊的好友:>\033[0m");
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
            handlechat(input);
            flag = true;
            break;
        }
        case '[':{
            list('[');
            if(us.friendlist.size())
                printf("\033[0;32m请选择您要私聊的好友:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            list(']');
            if(us.friendlist.size())
                printf("\033[0;32m请选择您要私聊的好友:>\033[0m");
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

void friendfucs::shield(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    //找到对应消息
    int i = 5*page + c - '0' - 1, j = 0;
    if(fg == 1)
        if(i >= us.friendlist.size()) return;
    else if(fg == 2)
        if(i >= fnl.data.size()) return;    
    std::string sd;
    if(fg == 1){
        for(std::string str : us.friendlist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    } else if(fg == 2){
        for(std::string str : fnl.data){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    sock->sendMsg("gtnm:"+sd);
    std::string nm = EchoMsgQueue.wait_and_pop(), rev;
    printf("\033[0;32m确定要屏蔽好友\033[0m \033[0;31m%s\033[0m \033[0;32m？（Y/N）\033[0m\n", nm.c_str());
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
    //确定屏蔽该好友,uid1屏蔽uid2
    sock->sendMsg("shfd:"+us.uid+":"+sd);
    rev = EchoMsgQueue.wait_and_pop();
    if(rev == "sheld"){
        printf("\033[0;31m已经屏蔽过该用户，无需重新屏蔽。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        input = charget();
        return ;
    }
    else if(rev != "right"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        input = charget();
        return ;
    }
    //屏蔽本地列表中的uid2
    us.shieldlist.insert(sd);
    system("clear");
    printf("\033[0;32m屏蔽成功。\n\033[0m");
    printf("\033[0;32m请按任意键继续...\033[0m");
    charget();
}

void friendfucs::shield_menu(char c){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if(!us.shieldlist.size()){
        system("clear");
        printf("\033[0;32m当前黑名单为空。\n\033[0m");
        printf("\033[0;32m请按ESC返回...\033[0m");
        return;
    }
    int cnt = 0;
    cnt = us.shieldlist.size();
    int maxpage = cnt / 5, i = 0;
    if(cnt % 5 != 0) maxpage++;
    if(c == '[' && page == 0) ;
    else if(c == '[') page --;
    if(c == ']' && page+1 >= maxpage) ;
    else if(c == ']') page ++;
    system("clear");
    printf("\033[0;36m==========================================================\033[0m\n");
    reportfucs::newreport(us, clientp);
    printf("\033[0;32m以下为您的黑名单\033[0m\n");
    printf("\033[0;34m%-6s %-15s %-13s %-12s\033[0m\n", "序号", "用户名", "UID", "在线状态");
    for(std::string str : us.shieldlist){
        if(i >= 5*page && i < 5*(page+1)){
            sock->sendMsg("gtus:"+str);
            std::string red = EchoMsgQueue.wait_and_pop();
            user ud = user::fromJson(red);
            std::string name = ud.name, status;
            if(ud.stat == "online") status = "在线";
            else if(ud.stat == "offline") status = "离线";
            else if(ud.stat == "destroy") status = "该账户已注销";

            // 如果是在线，颜色绿色；否则灰色
            const char *color = (status == "在线") ? "\033[0;32m" : "\033[0;90m";

            printf("%s[%d]  %-12s %-14s %-12s\033[0m",
                    color, i - 5 * page + 1,
                    name.c_str(), ud.uid.c_str(), status.c_str());
            if(rpt.chatfriend[ud.uid]) printf("   \033[0;31m（%d）\033[0m\n", rpt.chatfriend[ud.uid]);
            else puts("");
        }
        i++;
    }
    printf("                     \033[0;32m(tip:按[和]按键可控制翻页)\n\033[0m");
    printf("                                \033[0;32m[%d/%d]\033[0m\n",page+1,maxpage);
    printf("\033[0;36m==========================================================\033[0m\n");
}

void friendfucs::shield_exit(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    //找到对应消息
    int i = 5*page + c - '0' - 1, j = 0;
    if((fg == 1 && i >= us.shieldlist.size()) || (fg == 2 && i >= fnl.data.size())) {
        system("clear");
        printf("\033[0;32m该用户不在黑名单中。\n\033[0m");
        printf("\033[0;32m请按任意键继续...\033[0m");
        charget();
        return;
    }
    std::string sd;
    if(fg == 1){
        for(std::string str : us.shieldlist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    } else if (fg == 2){
        for(std::string str : fnl.data){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    sock->sendMsg("gtnm:"+sd);
    std::string nm = EchoMsgQueue.wait_and_pop(), rev;
    printf("\033[0;32m确定要解除屏蔽用户\033[0m \033[0;31m%s\033[0m \033[0;32m？（Y/N）\033[0m\n", nm.c_str());
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
    //确定解除屏蔽该好友,uid1解除屏蔽uid2
    if(us.shieldlist.count(sd) == 0){//该好友不在黑名单中
        system("clear");
        printf("\033[0;32m该用户不在黑名单中。\n\033[0m");
        printf("\033[0;32m请按任意键继续...\033[0m");
        charget();
        return;
    }
    sock->sendMsg("shex:"+us.uid+":"+sd);
    rev = EchoMsgQueue.wait_and_pop();
    if(rev != "right"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        input = charget();
        return ;
    }
    //去除本地黑名单中的uid2
    us.shieldlist.erase(sd);
    for(int i = 0; i < fnl.data.size(); i++)
    if(fnl.data[i] == sd) fnl.data.erase(fnl.data.begin() + i);
    system("clear");
    printf("\033[0;32m解除屏蔽成功。\n\033[0m");
    printf("\033[0;32m请按任意键继续...\033[0m");
    charget();
}

void friendfucs::shieldfriend(int cs){
    page = 0;
    if(cs == 1){
        list('0');
        if(us.friendlist.size())
            printf("\033[0;32m请选择您要屏蔽的好友:>\033[0m");
    } else if(cs == 2){
        shield_menu('0');
        if(us.shieldlist.size())
            printf("\033[0;32m请选择您要解除屏蔽的用户:>\033[0m");
    }
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
            if(cs == 1){
                list('p');
                if(us.friendlist.size())
                    printf("\033[0;32m请选择您要屏蔽的好友:>\033[0m");
            } else if(cs == 2){
                shield_menu('0');
                if(us.shieldlist.size())
                    printf("\033[0;32m请选择您要解除屏蔽的用户:>\033[0m");
            }
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
            if(cs == 1 && us.friendlist.size())
                shield(input);
            else if(cs == 2 && us.shieldlist.size())
                shield_exit(input);
            flag = true;
            break;
        }
        case '[':{
            if(cs == 1){
                list('[');
                if(us.friendlist.size())
                    printf("\033[0;32m请选择您要屏蔽的好友:>\033[0m");
            } else if(cs == 2){
                shield_menu('0');
                if(us.shieldlist.size())
                    printf("\033[0;32m请选择您要解除屏蔽的用户:>\033[0m");
            }
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            if(cs == 1){
                list(']');
                if(us.friendlist.size())
                    printf("\033[0;32m请选择您要屏蔽的好友:>\033[0m");
            } else if(cs == 2){
                shield_menu('0');
                if(us.shieldlist.size())
                    printf("\033[0;32m请选择您要解除屏蔽的用户:>\033[0m");
            }
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

void friendfucs::searchlist(char c){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if(!fnl.data.size()){
        system("clear");
        printf("\033[0;32m没有包含该字段的好友。\n\033[0m");
        printf("\033[0;32m请按ESC返回...\033[0m");
        return;
    }
    int cnt = 0;
    cnt = fnl.data.size();
    int maxpage = cnt / 5, i = 0;
    if(cnt % 5 != 0) maxpage++;
    if(c == '[' && page == 0) ;
    else if(c == '[') page --;
    if(c == ']' && page+1 >= maxpage) ;
    else if(c == ']') page ++;
    system("clear");
    printf("\033[0;36m==========================================================\033[0m\n");
    reportfucs::newreport(us, clientp);
    printf("\033[0;32m以下为符合条件的好友\033[0m\n");
    printf("\033[0;34m%-6s %-15s %-13s %-12s\033[0m\n", "序号", "用户名", "UID", "在线状态");
    for(std::string str : fnl.data){
        if(i >= 5*page && i < 5*(page+1)){
            sock->sendMsg("gtus:"+str);
            std::string red = EchoMsgQueue.wait_and_pop();
            user ud = user::fromJson(red);
            std::string name = ud.name, status;
            if(ud.stat == "online") status = "在线";
            else if(ud.stat == "offline") status = "离线";
            else if(ud.stat == "destroy") status = "该账户已注销";

            // 如果是在线，颜色绿色；否则灰色
            const char *color = (status == "在线") ? "\033[0;32m" : "\033[0;90m";

            printf("%s[%d]  %-12s %-14s %-12s\033[0m",
                    color, i - 5 * page + 1,
                    name.c_str(), ud.uid.c_str(), status.c_str());
            if(rpt.chatfriend[ud.uid]) printf("   \033[0;31m（%d）\033[0m\n", rpt.chatfriend[ud.uid]);
            else puts("");
        }
        i++;
    }
    printf("                     \033[0;32m(tip:按[和]按键可控制翻页)\n\033[0m");
    printf("                                \033[0;32m[%d/%d]\033[0m\n",page+1,maxpage);
    printf("\033[0;36m==========================================================\033[0m\n");
}

void friendfucs::searchfriend(){
    Client * cp = (Client*)clientp;
    Socket* sock = cp->getSocket();
    std::string str;
    char name[512];
    char sd[550];
    system("clear");
    printf("\033[0;32m请输入您要搜索的好友名:\033[0m\n>");

    while(1){
        chu(name);
        int ret = enter(name, 0);
        if(ret == -1) return;
        if(name[0] == '\n' || name[0] == '\0' || name[0] == ' ') {
            printf("\n>");
            continue;
        }
        //获取好友名列表
        sprintf( sd, "fdlt:%s:%s", us.uid.c_str(), name);
        sock->sendMsg(sd);
        str = EchoMsgQueue.wait_and_pop();
        break;
    }
    fnl = friendnamelist::fromJson(str);
    if(!fnl.data.size()){
        printf("\033[0;32m没有包含该字段的好友。\n\033[0m");
        printf("\033[0;32m请按任意键继续...\033[0m");
        charget();
        return;
    }
    page = 0;
    searchlist('0');
    if(fnl.data.size())
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
            searchlist('p');
            if(fnl.data.size())
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
            if(p >= 0 && p < fnl.data.size()){
                select(input, 2);
                flag = true;
            }
            break;
        }
        case '[':{
            searchlist('[');
            if(fnl.data.size())
                printf("\033[0;32m输入序号可进行操作:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            searchlist(']');
            if(fnl.data.size())
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