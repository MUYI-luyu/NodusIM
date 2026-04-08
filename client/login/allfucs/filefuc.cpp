#include "./filefuc.h"

std::string server_ip = "10.30.0.109";
int server_port = CONPORT;

std::atomic<bool> file_sending(false);
std::atomic<bool> file_recving(false);
Client dataclient;
FILE* file;
char arr[1024];
char path[3072];
std::string fid;
std::string sd;
std::string uuid;

std::string filefucs::GetFileName(const char arr[]){
  int i = strlen(arr) - 1;
  std::string str;
  for(;i >= 0; i--){
    if(arr[i] == '/') break;
    else str.push_back(arr[i]);
  }
  std::reverse(str.begin(), str.end());
  return str;
}

bool filefucs::conntect_filepth(){
    if (!dataclient.connectToHost(server_ip.c_str(), 5513)) {
        return false;
    }
    Socket* sock = dataclient.getSocket();
    sock->setNonBlocking();
    return true;
}


void filefucs::sendfile_touser(char c, int fg){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    if(file_sending){
        printf("\033[0;31m当前还有文件未传输完，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    }

    //找到对应消息
    int i = 5*page + c - '0' - 1, j = 0, ret;
    if(fg == 1){
        if(i >= us.friendlist.size()) return;
        for(std::string str : us.friendlist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    } else if(fg == 2){
        if(i >= us.grouplist.size()) return;
        for(std::string str : us.grouplist){
            if(j == i){
                sd = str;
                break;
            }
            j++;
        }
    }
    uuid = us.uid;
    //下载文件
    //输出指引
    printf("\033[0;32m输入需要传输的文件路径:>\033[0m");
    while(1){
        fflush(stdout); // 手动刷新标准输出缓冲区

        memset(arr,0,sizeof arr);
        //获取输入文件名
        ret = read(STDIN_FILENO,arr,sizeof arr);
        if(ret <= 0){
            printf("输入错误:%s\n",strerror(errno));
            exit(1);
        }
        if(arr[0] == '\n') {
            system("clear");
            printf("\033[0;32m输入包含ESC的语句可回到上一级页面。\n路径有误,请重新输入:>\033[0m");
            continue;
        }
        arr[ret - 1] = '\0';
        char *pa = NULL;
        pa = getcwd(NULL, 0);
        //若不为绝对路径，拼接路径
        if(arr[0]!='/')
            sprintf(path,("%s/%s"),pa,arr);
        //若为绝对路径，直接传入路径
        else strcpy(path,arr);
        if (pa) free(pa);
        file = fopen(path, "rb");
        if (file == NULL) {
            system("clear");
            for(int i = 0; i < strlen(arr); i++) if(arr[i] == 27) return;
            printf("\033[0;32m输入包含ESC的语句可回到上一级页面。\n路径有误,请重新输入:>\033[0m");
            continue;
        }
        else break;
    }
    printf("输入成功:%s\n",arr);
    printf("正在上传文件...\n");
    //获取fid
    sock->sendMsg("gtfi:"+us.uid+":"+path);
    fid = EchoMsgQueue.wait_and_pop();

    //获取数据传输套接字
    Socket* datasock = dataclient.getSocket();
    //发送文件名
    // str.clear();
    // str = GetFileName(arr);
    // datasock->sendMsg("sdfl:"+us.uid+":"+sd+":"+fid +":"+str);

    //获取文件状态
    struct stat st;
    if(lstat(path,&st) == -1){//获取文件状态
        perror("stat");//处理错误返回值
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    }
    //std::thread sendfilepth(&filefucs::upload_file_with_offset, this);
    if(fg == 1){
        std::thread sendfilepth(std::bind(&filefucs::upload_file_with_offset, this));
        sendfilepth.detach();  //后台运行，不阻塞主线程
    } else if(fg == 2){
        std::thread sendfilepth(std::bind(&filefucs::upload_gfile_with_offset, this));
        sendfilepth.detach();  //后台运行，不阻塞主线程
    }

    printf("\033[0;31m正在后台发文件，请按任意键继续...\033[0m");
    charget();
    return;
}

void filefucs::upload_file_with_offset() {
    file_sending = true;
    Socket* datasock = dataclient.getSocket();

    const size_t block_size = 4 * 1024 * 1024; // 4MB
    std::vector<char> buf(block_size); // 用堆分配避免栈溢出

    long long offset = 0;
    size_t bytesRead;
    file_block block;

    while ((bytesRead = fread(buf.data(), 1, block_size, file)) > 0) {
        // 构造 file_block
        block.sender_uid = uuid;
        block.receiver_uid = sd;
        block.fid = fid;
        block.filename = arr;
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

        if (datasock->sendFILE("rvfl:" + packet) == -1) {
            printf("发送失败，连接异常\n");
            fclose(file);
            file_sending = false;
            return;
        }

        offset += bytesRead;
    }

    fclose(file);
    datasock->sendMsg("fled:" + block.toJson());  // 通知结束
    file_sending = false;
}

void filefucs::upload_gfile_with_offset() {
    file_sending = true;
    Socket* datasock = dataclient.getSocket();

    const size_t block_size = 4 * 1024 * 1024; // 4MB
    std::vector<char> buf(block_size); // 避免大数组占用栈空间

    off_t offset = 0;
    size_t bytesRead;
    file_block block;

    while ((bytesRead = fread(buf.data(), 1, block_size, file)) > 0) {
        // 构造 file_block
        block.sender_uid = uuid;
        block.receiver_uid = sd;
        block.fid = fid;
        block.filename = arr;
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

        if (datasock->sendFILE("rvgf:" + packet) == -1) {
            printf("发送失败，连接异常\n");
            fclose(file);
            file_sending = false;
            return;
        }

        offset += bytesRead;
    }

    fclose(file);
    datasock->sendMsg("gfed:" + block.toJson());  // 通知结束
    file_sending = false;
}


void filefucs::download_file_with_offset(std::string sd){
    std::string buf;
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    file_recving = true;
    bool sendfileok = false, first = true;
    Socket* datasock = dataclient.getSocket();
    datasock->setNonBlocking();
    datasock->sendMsg("sdfl:"+uuid+":"+sd);
    std::string ret = EchoMsgQueue.wait_and_pop(), packet;
    if(ret == "error"){
        printf("接收失败，连接异常\n");
        file_recving = false;
        return;
    }
    FILE* f;
    while(1){
        if(sendfileok && !buf.size()) break;
        datasock->recvfull(buf);
        while(1){ //4*1024*1024
            //printf("buf.size():%ld\n", buf.size());
            if(buf.size() >= 4){
                uint32_t len, slen;
                std::memcpy(&len, buf.data(), sizeof(len));
                slen = ntohl(len);
                if(buf.size() < 4 + slen) {
                    //printf("buf.size():%ld < 4 + slen%d\n", buf.size(), slen);
                    break;
                }
                packet = buf.substr(4, slen);
                buf.erase(0, 4+slen);
                //printf("取出slen:%d, packet,size():%ld, packet:%s\n", slen,packet.size(),packet.c_str());
                if(packet == "end") {
                    sendfileok = true;
                    fclose(f);
                    datasock->sendMsg("rved:"+uuid+":"+sd);  // 通知结束
                    file_recving = false;
                    return;
                }
            }
            else break;
            //printf("read packetsize%ld, str:%s\n", packet.size() ,packet.c_str());
            //处理
            if (packet.size() < sizeof(uint32_t)) {
                printf("接收失败，连接异常\n");
                if(!first) fclose(f);
                file_recving = false;
                return;
            }
            //1. 提取前4字节，得到 JSON 字符串长度
            uint32_t json_len;
            std::memcpy(&json_len, packet.data(), sizeof(uint32_t));
            json_len = ntohl(json_len);  // 网络字节序转主机序
            //2. 检查总长度是否合法
            if (packet.size() < sizeof(uint32_t) + json_len) {
                printf("数据包长度不足，缺失 JSON 部分\n");
                return;
            }
            //3. 提取 JSON 字符串
            std::string json_str = packet.substr(sizeof(uint32_t), json_len);
            //4. 解析 JSON
            file_block fb = file_block::fromJson(json_str);
            //5. 提取数据部分
            size_t data_offset = sizeof(uint32_t) + json_len;
            std::string data = packet.substr(data_offset);

            if(first){
                //构造目录路径: ./recvfile/file_<recver_uid>/
                std::string dir_path = "./recvfile/file_" + fb.receiver_uid;
                std::filesystem::create_directories(dir_path);  // 若已存在不会报错

                //构造文件名: sender_uid:fid:filename
                std::string file_name = GetFileName(fb.filename.data());
                std::string full_path = dir_path + "/" + file_name;

                //写入文件
                f = fopen(full_path.c_str(), "r+b");
                if (!f) {
                    f = fopen(full_path.c_str(), "wb");
                    if (!f) {
                        perror("无法创建文件");
                        return;
                    }
                }
                first = false;
            }
            fseek(f, fb.offset, SEEK_SET);
            fwrite(data.data(), 1, data.size(), f);
        }
    }

    fclose(f);
    datasock->sendMsg("rved:"+uuid+":"+sd);  // 通知结束
    file_recving = false;
}


void filefucs::download_gfile_with_offset(std::string sd){
    std::string buf;
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    file_recving = true;
    bool sendfileok = false, first = true;
    Socket* datasock = dataclient.getSocket();
    datasock->setNonBlocking();
    datasock->sendMsg("sdgf:"+uuid+":"+sd);
    std::string ret = EchoMsgQueue.wait_and_pop(), packet;
    if(ret == "error"){
        printf("接收失败，连接异常\n");
        file_recving = false;
        return;
    }
    FILE* f;
    while(1){
        if(sendfileok && !buf.size()) break;
        datasock->recvfull(buf);
        while(1){
            //printf("buf.size():%ld\n", buf.size());
            if(buf.size() >= 4){
                uint32_t len, slen;
                std::memcpy(&len, buf.data(), sizeof(len));
                slen = ntohl(len);
                if(buf.size() < 4 + slen) {
                    //printf("buf.size():%ld < 4 + slen%d\n", buf.size(), slen);
                    break;
                }
                packet = buf.substr(4, slen);
                buf.erase(0, 4+slen);
                //printf("取出slen:%d, packet,size():%ld, packet:%s\n", slen,packet.size(),packet.c_str());
                if(packet == "end") {
                    sendfileok = true;
                    fclose(f);
                    datasock->sendMsg("rvge:"+uuid+":"+sd);  // 通知结束
                    file_recving = false;
                    return;
                }
            }
            else break;
            //printf("read packetsize%ld, str:%s\n", packet.size() ,packet.c_str());
            //处理
            if (packet.size() < sizeof(uint32_t)) {
                printf("接收失败，连接异常\n");
                if(!first) fclose(f);
                file_recving = false;
                return;
            }
            //1. 提取前4字节，得到 JSON 字符串长度
            uint32_t json_len;
            std::memcpy(&json_len, packet.data(), sizeof(uint32_t));
            json_len = ntohl(json_len);  // 网络字节序转主机序
            //2. 检查总长度是否合法
            if (packet.size() < sizeof(uint32_t) + json_len) {
                printf("数据包长度不足，缺失 JSON 部分\n");
                return;
            }
            //3. 提取 JSON 字符串
            std::string json_str = packet.substr(sizeof(uint32_t), json_len);
            //4. 解析 JSON
            file_block fb = file_block::fromJson(json_str);
            //5. 提取数据部分
            size_t data_offset = sizeof(uint32_t) + json_len;
            std::string data = packet.substr(data_offset);

            if(first){
                //构造目录路径: ./recvfile/gfile_<recver_uid>/
                std::string dir_path = "./recvfile/gfile_" + fb.receiver_uid;
                std::filesystem::create_directories(dir_path);  // 若已存在不会报错

                //构造文件名: sender_uid:fid:filename
                std::string file_name = fb.filename;
                std::string full_path = dir_path + "/" + file_name;

                //写入文件
                f = fopen(full_path.c_str(), "r+b");
                if (!f) {
                    f = fopen(full_path.c_str(), "wb");
                    if (!f) {
                        perror("无法创建文件");
                        return;
                    }
                }
                first = false;
            }

            fseek(f, fb.offset, SEEK_SET);
            fwrite(data.data(), 1, data.size(), f);
        }
    }

    fclose(f);
    datasock->sendMsg("rvge:"+uuid+":"+sd);  // 通知结束
    file_recving = false;
}


void filefucs::list(char c, int fg){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if(fg == 1){
        if(!us.friendlist.size()){
            printf("\033[0;32m当前没有好友。\n\033[0m");
            printf("\033[0;32m请按ESC返回...\033[0m");
            return;
        }
    } else if(fg == 2){
        if(!us.grouplist.size()){
            printf("\033[0;32m当前没有加入的群聊。\n\033[0m");
            printf("\033[0;32m请按ESC返回...\033[0m");
            return;
        }
    }
    int cnt = 0;
    if(fg == 1)
        cnt = us.friendlist.size();
    else if(fg == 2)
        cnt = us.grouplist.size();
    int maxpage = cnt / 5, i = 0;
    if(cnt % 5 != 0) maxpage++;
    if(c == '[' && page == 0) ;
    else if(c == '[') page --;
    if(c == ']' && page+1 >= maxpage) ;
    else if(c == ']') page ++;
    printf("\033[0;36m==========================================================\033[0m\n");
    reportfucs::newreport(us, clientp);
    if(fg == 1){
        printf("\033[0;32m以下为您的好友\033[0m\n");
        printf("\033[0;34m%-6s %-15s %-13s %-12s\033[0m\n", "序号", "用户名", "UID", "在线状态");
    }
    else if(fg == 2){
        printf("\033[0;32m以下为您加入的群聊\033[0m\n");
        printf("\033[0;34m%-6s %-15s %-13s %-12s\033[0m\n", "序号", "群聊名", "GID", "在群中身份");
    }
    if(fg == 1){
        for(std::string str : us.friendlist){
            if(i >= 5*page && i < 5*(page+1)){
                sock->sendMsg("gtus:"+str);
                std::string red = EchoMsgQueue.wait_and_pop();
                user ud = user::fromJson(red);
                std::string name = ud.name, status;
                if(ud.stat == "online") status = "在线";
                else if(ud.stat == "offline") status = "离线";
                else if(ud.stat == "deleted") status = "该账户已注销";

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
    } else if(fg == 2){
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
    }
    printf("                     \033[0;32m(tip:按[和]按键可控制翻页)\n\033[0m");
    printf("                                \033[0;32m[%d/%d]\033[0m\n",page+1,maxpage);
    printf("\033[0;36m==========================================================\033[0m\n");
}


void filefucs::listfriend(){
    system("clear");
    page = 0;
    list('0');
    if(us.friendlist.size())
        printf("\033[0;32m选择您要传文件的好友:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false, sendfileok = false;
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
        if(flag || sendfileok){
            if(sendfileok){
                dataclient.reinitialize();
                conntect_filepth();
                sendfileok = false;
            }
            flag = false;
            system("clear");
            list('p');
            if(us.friendlist.size())
                printf("\033[0;32m选择您要传文件的好友:>\033[0m");
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
                sendfile_touser(input);
                flag = true;
                sendfileok = true;
            }
            break;
        }
        case '[':{
            system("clear");
            list('[');
            if(us.friendlist.size())
                printf("\033[0;32m选择您要传文件的好友:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            list(']');
            if(us.friendlist.size())
                printf("\033[0;32m选择您要传文件的好友:>\033[0m");
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


void filefucs::listgroup(){
    system("clear");
    page = 0;
    list('0', 2);
    if(us.grouplist.size())
        printf("\033[0;32m选择您要传文件的群聊:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false, sendfileok = false;
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
        if(flag || sendfileok){
            if(sendfileok){
                dataclient.reinitialize();
                conntect_filepth();
                sendfileok = false;
            }
            flag = false;
            system("clear");
            list('p', 2);
            if(us.grouplist.size())
                printf("\033[0;32m选择您要传文件的群聊:>\033[0m");
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
            if(p >= 0 && p < us.grouplist.size()){
                sendfile_touser(input, 2);
                flag = true;
                sendfileok = true;
            }
            break;
        }
        case '[':{
            system("clear");
            list('[', 2);
            if(us.grouplist.size())
                printf("\033[0;32m选择您要传文件的群聊:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            list(']', 2);
            if(us.grouplist.size())
                printf("\033[0;32m选择您要传文件的群聊:>\033[0m");
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


void filefucs::filelist(char c, int fg){
    Client* cp = (Client*) clientp;
    Socket* sock = cp->getSocket();
    if(!fnl.data.size()){
        printf("\033[0;32m当前没有对您发送的文件。\n\033[0m");
        printf("\033[0;32m请按ESC返回...\033[0m");
        return;
    }
    int cnt = fnl.data.size();
    int maxpage = cnt / 5, i = 0;
    if(cnt % 5 != 0) maxpage++;
    if(c == '[' && page == 0) ;
    else if(c == '[') page --;
    if(c == ']' && page+1 >= maxpage) ;
    else if(c == ']') page ++;
    printf("\033[0;36m==========================================================\033[0m\n");
    reportfucs::newreport(us, clientp);
    if(fg == 1){
        printf("\033[0;32m以下为您的可接收的文件\033[0m\n");
        printf("\033[0;34m%-6s %-15s %-13s %-12s\033[0m\n", "序号", "发送人", "FID", "文件名");
    }
    else if(fg == 2){
        printf("\033[0;32m以下为您的可接收的群文件\033[0m\n");
        printf("\033[0;34m%-6s %-15s %-13s %-12s\033[0m\n", "序号", "来源群聊", "FID", "文件名");
    }
    for(std::string str : fnl.data){
        if(i >= 5*page && i < 5*(page+1)){
            //拿到数据
            std::string uid2, fid, filename;
            int m = 0, t, cnt;
            while(str[m] != ':') uid2.push_back(str[m++]);
            int j = m + 1;
            while(str[j] != ':') fid.push_back(str[j++]);
            for(int k = j + 1; k < str.size(); k++) filename.push_back(str[k]);
            if(fg == 1){
                sock->sendMsg("gtus:"+uid2);
                std::string red = EchoMsgQueue.wait_and_pop();
                user ud = user::fromJson(red);
                std::string name = ud.name, status;
                if(ud.stat == "online") status = "在线";
                else if(ud.stat == "offline") status = "离线";
                else if(ud.stat == "deleted") status = "该账户已注销";

                // 如果是在线，颜色绿色；否则灰色
                const char *color = (status == "在线") ? "\033[0;32m" : "\033[0;90m";
                printf("%s[%d]  %-12s %-13s %-12s\033[0m\n",
                        color, i - 5 * page + 1,
                        name.c_str(), fid.c_str(), filename.c_str());
            } else if(fg == 2){
                sock->sendMsg("gtgp:"+uid2);
                std::string red = EchoMsgQueue.wait_and_pop();
                if(red == "norepeat") continue;
                group gp = group::fromJson(red);
                std::string name = gp.name;
                const char *color = "\033[0;32m";
                printf("%s[%d]  %-12s %-13s %-12s\033[0m\n",
                        color, i - 5 * page + 1,
                        name.c_str(), fid.c_str(), filename.c_str());
            }
        }
        i++;
    }
    printf("                     \033[0;32m(tip:按[和]按键可控制翻页)\n\033[0m");
    printf("                                \033[0;32m[%d/%d]\033[0m\n",page+1,maxpage);
    printf("\033[0;36m==========================================================\033[0m\n");
}


std::string filefucs::formatFileSize(uint64_t size_bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    double size = static_cast<double>(size_bytes);
    int unit_index = 0;

    while (size >= 1024.0 && unit_index < 5) {
        size /= 1024.0;
        ++unit_index;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << ' ' << units[unit_index];
    return oss.str();
}


void filefucs::download(char c, int fg = 1){
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    if(file_recving){
        printf("\033[0;31m当前还有文件未接收完，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    }

    //找到对应消息
    int i = 5*page + c - '0' - 1, j = 0, ret;
    if(i >= fnl.data.size()) return;
    for(std::string str : fnl.data){
        if(j == i){
            sd = str;
            break;
        }
        j++;
    }
    //解析数据
    std::string uid2, fid, filename;
    int m = 0, t, cnt;
    long stsize = 0;
    while(sd[m] != ':') uid2.push_back(sd[m++]);
    j = m + 1;
    while(sd[j] != ':') fid.push_back(sd[j++]);
    for(int k = j + 1; k < sd.size(); k++) filename.push_back(sd[k]);
    //获取文件大小
    if(fg == 1)
        sock->sendMsg("flsz:"+ sd);
    else if(fg == 2)
        sock->sendMsg("gfsz:"+ sd);
    std::string sz = EchoMsgQueue.wait_and_pop();
    if(sz == "error"){
        printf("\033[0;31m数据异常，请稍后再试。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m");
        charget();
        return ;
    }
    sscanf(sz.c_str(), "%ld", &stsize);
    printf("\033[0;32m文件名：\033[0m%s\n", filename.c_str());
    printf("\033[0;32m文件id：\033[0m%s\n", fid.c_str());
    printf("\033[0;32m文件大小：\033[0m%s\n", formatFileSize(stsize).c_str());
    printf("\033[0;32m确定要接收该文件？（Y/N）\033[0m\n");
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
    uuid = us.uid;
    if(fg == 1){
        std::thread sendfilepth(std::bind(&filefucs::download_file_with_offset, this, sd));
        sendfilepth.detach();  //后台运行，不阻塞主线程
    } else if (fg == 2){
        std::thread sendfilepth(std::bind(&filefucs::download_gfile_with_offset, this, sd));
        sendfilepth.detach();  //后台运行，不阻塞主线程
    }
    printf("\033[0;31m正在后台接收文件，请按任意键继续...\033[0m");
    charget();
}


void filefucs::downloadfile(){
    //向服务器获取数据
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    printf("\033[0;32m数据请求中...\033[0m\n");
    sock->sendMsg("gtrl:"+us.uid);
    std::string frl = EchoMsgQueue.wait_and_pop();
    if(frl == "nofile"){
        system("clear");
        printf("\033[0;31m当前没有好友对您发送的文件。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m\n");
        charget();
        return;
    }
    fnl = friendnamelist::fromJson(frl);

    system("clear");
    page = 0;
    filelist('0', 1);
    if(fnl.data.size())
        printf("\033[0;32m请选择您要接收的文件:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false, recvfileok = false;
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
        if(flag || recvfileok){
            if(recvfileok){
                dataclient.reinitialize();
                conntect_filepth();
                recvfileok = false;
            }
            flag = false;
            system("clear");
            filelist('p', 1);
            if(fnl.data.size())
                printf("\033[0;32m请选择您要接收的文件:>\033[0m");
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
                download(input);
                flag = true;
                recvfileok = true;
            }
            break;
        }
        case '[':{
            system("clear");
            filelist('[', 1);
            if(fnl.data.size())
                printf("\033[0;32m请选择您要接收的文件:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            filelist(']', 1);
            if(fnl.data.size())
                printf("\033[0;32m请选择您要接收的文件:>\033[0m");
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


void filefucs::downloadgfile(){
    //向服务器获取数据
    Client * cp = (Client*)clientp;
    Socket * sock = cp->getSocket();
    system("clear");
    printf("\033[0;32m数据请求中...\033[0m\n");
    sock->sendMsg("gtgf:"+us.uid);
    std::string frl = EchoMsgQueue.wait_and_pop();
    if(frl == "nofile"){
        system("clear");
        printf("\033[0;31m当前没有可接收的群文件。\033[0m\n");
        printf("\033[0;31m请按任意键继续...\033[0m\n");
        charget();
        return;
    }
    fnl = friendnamelist::fromJson(frl);

    system("clear");
    page = 0;
    filelist('0', 2);
    if(fnl.data.size())
        printf("\033[0;32m请选择您要接收的文件:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
    bool flag = false, recvfileok = false;
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
        if(flag || recvfileok){
            if(recvfileok){
                dataclient.reinitialize();
                conntect_filepth();
                recvfileok = false;
            }
            flag = false;
            system("clear");
            filelist('p', 2);
            if(fnl.data.size())
                printf("\033[0;32m请选择您要接收的文件:>\033[0m");
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
                download(input, 2);
                flag = true;
                recvfileok = true;
            }
            break;
        }
        case '[':{
            system("clear");
            filelist('[', 2);
            if(fnl.data.size())
                printf("\033[0;32m请选择您要接收的文件:>\033[0m");
            fflush(stdout); // 手动刷新标准输出缓冲区
            break;
        }
        case ']':{
            system("clear");
            filelist(']', 2);
            if(fnl.data.size())
                printf("\033[0;32m请选择您要接收的文件:>\033[0m");
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