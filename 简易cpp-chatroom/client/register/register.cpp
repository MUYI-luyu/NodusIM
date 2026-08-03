#include "./register.h"

//注册功能逻辑函数
void Register::rgst(void * p){
    Client* client = (Client*) p;
    Socket* sock = client->getSocket();

    system("clear");
    printf("\033[0;32m已选择注册选项(按下ESC可返回上一级菜单)\033[0m\n");
    printf("\033[0;32m请输入您要注册的用户名(仅允许英文字母、数字和汉字的组合)\n\033[0m>");

    //输入用户名
    do{
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
            printf("\033[0;31m用户名仅允许英文字母、数字和汉字，请重新输入。\n\033[0m>");
            continue;
        }
        //判断用户名合法性
        int tmp = is_valid_username(name);
        if(tmp == -1){
            printf("\033[0;31m用户名仅允许英文字母、数字和汉字，请重新输入。\n\033[0m>");
            continue;
        }
        if(tmp <= 3) {
            printf("\033[0;31m用户名至少为4位，请重新输入。\n\033[0m>");
            continue;
        }
        //判断是否有重名
        //询问服务器
        sock->sendMsg("jrnm:"+std::string(name));//judge_repeat_name
        //sock->sendMsg(name);
        std::string str;
        str = EchoMsgQueue.wait_and_pop();
        if(strcmp(str.c_str(), "echo:norepeat") != 0){
            printf("\033[0;31m用户名已存在，请重新输入。\n\033[0m>");
            continue;
        }
        break;
    }while(1);


    //输入密码
    printf("\033[0;32m请输入您的密码\n\033[0m>");
    do{
        chu(pwd);
        int ret = enter(pwd, 1);
        if(ret == -1) return;
        else if(ret <= 6) {
            printf("\033[0;31m密码至少为7位，请重新输入。\n\033[0m>");
            continue;
        }
        int key = 0;
        for(int i = 0; i < ret; i++){
            if(!((pwd[i] >= 'a' && pwd[i] <= 'z')||(pwd[i] >= 'A' && pwd[i] <= 'Z')||(pwd[i] >= '0' && pwd[i] <= '9'))){
                int j;
                for(j = 0; j < pmitchar.size(); j++){
                    if(pmitchar[j] == pwd[i]) break;
                }
                if(j == pmitchar.size()){
                    key = 1;
                    printf("\033[0;31密码仅能含有数字、字母和大部分键盘可打出的字符，当前密码不合法，请重新输入。\n\033[0m>");
                    break;
                }
            }
        }
        if(key) continue;
        break;
    } while(1);
    
    //再次输入密码
    printf("\033[0;32m请再次输入您的密码\n\033[0m>");
    char pwd2[256];
    do{
        chu(pwd2);
        int ret = enter(pwd2, 1);
        if(ret == -1) return;
        if(strcmp(pwd,pwd2) != 0){
            printf("\033[0;31m输入与第一次不符，请重新输入。\n\033[0m>");
            continue;
        }
        break;
    } while(1);

    //哈希密码,加密数据
    PasswordHasher hs;
    hs.hashPassword(pwd);

    // 注册不再要求邮箱和验证码，保留空字段以兼容现有用户 JSON 格式。

    //注册成功，发给服务器，写入数据库
    user u;
    u.uid = "0";
    u.name = name;
    u.pwd = pwd;
    u.email = "";
    u.stat = "offline";
    u.friendlist = {};
    u.grouplist = {};
    u.shieldlist = {};
    std::string datastr = u.toJson();
    printf("\033[0;32m输入正确!正在注册账号...\n\033[0m");
    sock->sendMsg("rgst:"+datastr);
    std::string rev; 
    rev = EchoMsgQueue.wait_and_pop();
    system("clear");
    if(rev == "fail") printf("\033[0;31m注册失败，请稍后再试:)\n\033[0m");
    else{
        printf("\033[0;32m注册成功!\n\033[0m");
        printf("\033[0;32m以下为您的基本信息:\n\033[0m");
        printf("\033[0;32m用户名:\033[0m%s\n",name);
        printf("\033[0;32muid:\033[0m%s\n",rev.c_str());
    }
    printf("\033[0;32m请按任意键继续...\033[0m");
    char rub;
    rub = charget();

    /*printf("user:%s\n",name);
    printf("pwd:%s\n",pwd2);
    printf("hashpwd:%s\n",pwd);
    printf("(pwd,hashpwd):%d\n",hs.verifyPassword(pwd2, pwd));
    printf("email:%s\n",email);
    sleep(1000);*/
}