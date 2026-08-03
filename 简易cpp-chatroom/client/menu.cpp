#include "menu.h"

void startmenu(void){
  printf("\033[0;36m==========================================================\033[0m\n");
  printf("           \033[0;33m欢迎进入聊天室,请选择要进行的操作\033[0m\n");
  printf("       \033[0;32m------------1.登录------------\033[0m\n");
  printf("       \033[0;32m------------2.注册------------\033[0m\n");
  printf("       \033[0;32m-----------3.找回密码----------\033[0m\n");
  printf("       \033[0;32m------------4.退出------------\033[0m\n");
  printf("\033[0;36m==========================================================\033[0m\n");

  printf("\033[0;32m请输入要选择功能的序号:>\033[0m");
  fflush(stdout); // 手动刷新标准输出缓冲区
}


void logmenu(){
    printf("\033[0;32m已选择登录选项(按下ESC可返回上一级菜单)\033[0m\n");
    printf("\033[0;32m请选择您的登录方式:\n\033[0m");
    printf("\033[0;33m1.用户名、密码登录\n\033[0m");
    printf("\033[0;33m2.邮箱、验证码登录\n\033[0m");
    printf("\033[0;32m请输入要选择的序号:>\033[0m");
    fflush(stdout); // 手动刷新标准输出缓冲区
}

void usermainmenu(user &u, void *p){
  system("clear");
  fflush(stdout); // 手动刷新标准输出缓冲区
  printf("\033[0;32m数据请求中...\033[0m\n");
  fflush(stdout); // 手动刷新标准输出缓冲区
  system("clear");
  printf("\033[0;36m==========================================================\033[0m\n");
  printf("                \033[0;33m用户%s,欢迎使用聊天室\033[0m\n", u.name.c_str());
  reportfucs::newreport(u, p);
  printf("\033[0;32m[1] 好友功能\033[0m\n");
  printf("\033[0;32m[2] 群聊功能\033[0m\n");
  printf("\033[0;32m[3] 文件功能\033[0m\n");
  printf("\033[0;32m[4] 查看通知\033[0m\n");
  printf("\033[0;32m[5] 设置与退出\033[0m\n");
  printf("\033[0;36m==========================================================\033[0m\n");

  printf("\033[0;32m请输入要选择功能的序号:>\033[0m");
  fflush(stdout); // 手动刷新标准输出缓冲区
}

void friendmenu(user &u, void *p){
  system("clear");
  fflush(stdout); // 手动刷新标准输出缓冲区
  printf("\033[0;32m数据请求中...\033[0m\n");
  system("clear");
  printf("\033[0;36m==========================================================\033[0m\n");
  printf("                \033[0;33m好友功能\033[0m\n");
  reportfucs::newreport(u, p);
  printf("\033[0;32m[1] 添加好友\033[0m\n");
  printf("\033[0;32m[2] 删除好友\033[0m\n");
  printf("\033[0;32m[3] 查看好友列表\033[0m\n");
  printf("\033[0;32m[4] 私聊好友\033[0m\n");
  printf("\033[0;32m[5] 屏蔽好友\033[0m\n");
  printf("\033[0;32m[6] 查看黑名单\033[0m\n");
  printf("\033[0;32m[7] 搜索好友\033[0m\n");
  printf("                    \033[0;33m(tip:ESC可返回上一级菜单)\033[0m\n");

  printf("\033[0;36m==========================================================\033[0m\n");

  printf("\033[0;32m请输入要选择功能的序号:>\033[0m");
  fflush(stdout); // 手动刷新标准输出缓冲区
}

void groupmenu(user& u, void *p){
  system("clear");
  fflush(stdout); // 手动刷新标准输出缓冲区
  printf("\033[0;32m数据请求中...\033[0m\n");
  fflush(stdout); // 手动刷新标准输出缓冲区
  system("clear");
  printf("\033[0;36m==========================================================\033[0m\n");
  printf("                \033[0;33m群聊功能\033[0m\n");
  reportfucs::newreport(u, p);
  printf("\033[0;32m[1] 创建群组\033[0m\n");
  printf("\033[0;32m[2] 加入群组\033[0m\n");
  printf("\033[0;32m[3] 查看已加入群组\033[0m\n");
  printf("\033[0;32m[4] 开始群聊\033[0m\n");
  printf("\033[0;32m[5] 退出群组\033[0m\n");
  printf("\033[0;32m[6] 查看群成员\033[0m\n");
  printf("\033[0;32m[7] 管理员操作\033[0m\n");
  printf("                    \033[0;33m(tip:ESC可返回上一级菜单)\033[0m\n");

  printf("\033[0;36m==========================================================\033[0m\n");

  printf("\033[0;32m请输入要选择功能的序号:>\033[0m");
  fflush(stdout); // 手动刷新标准输出缓冲区
}

void filemenu(user& u, void *p){
  system("clear");
  fflush(stdout); // 手动刷新标准输出缓冲区
  printf("\033[0;32m数据请求中...\033[0m\n");
  fflush(stdout); // 手动刷新标准输出缓冲区
  system("clear");
  printf("\033[0;36m==========================================================\033[0m\n");
  printf("                \033[0;33m文件功能\033[0m\n");
  reportfucs::newreport(u, p);
  printf("\033[0;32m[1] 给好友发送文件\033[0m\n");
  printf("\033[0;32m[2] 给群组发送文件\033[0m\n");
  printf("\033[0;32m[3] 下载好友发送的文件\033[0m\n");
  printf("\033[0;32m[4] 下载群文件\033[0m\n");
  printf("                    \033[0;33m(tip:ESC可返回上一级菜单)\033[0m\n");
  printf("\033[0;36m==========================================================\033[0m\n");

  printf("\033[0;32m请输入要选择功能的序号:>\033[0m");
}

void reportmenu(user& u, report rpt, bool ret){
  
  fflush(stdout); // 手动刷新标准输出缓冲区
  system("clear");
  printf("\033[0;36m==========================================================\033[0m\n");
  printf("                \033[0;33m查看通知\033[0m\n");

  printf("\033[0;32m[1] 查看好友申请      \033[0m");
  if(ret && rpt.friendapply.size()) printf("\033[0;31m(%ld)\033[0m\n", rpt.friendapply.size());
  else printf("\n");

  printf("\033[0;32m[2] 查看未读好友消息   \033[0m");
  if(ret && rpt.total_friend_msg) printf("\033[0;31m(%d)\033[0m\n", rpt.total_friend_msg);
  else printf("\n");

  printf("\033[0;32m[3] 查看未读群聊消息   \033[0m");
  if(ret && rpt.total_group_msg) printf("\033[0;31m(%d)\033[0m\n", rpt.total_group_msg);
  else printf("\n");

  printf("\033[0;32m[4] 查看加群申请      \033[0m");
  if(ret && rpt.groupapply.size()) printf("\033[0;31m(%ld)\033[0m\n", rpt.groupapply.size());
  else printf("\n");

  printf("\033[0;32m[5] 查看事务通知      \033[0m");
  int cnt = 0;
  for(int i = 0; i < rpt.notice.size(); i++)
    if(rpt.notice[i][5] == 'n') cnt++;
  if(ret && cnt) printf("\033[0;31m(%d)\033[0m\n", cnt);
  else printf("\n");

  printf("                    \033[0;33m(tip:ESC可返回上一级菜单)\033[0m\n");
  printf("\033[0;36m==========================================================\033[0m\n");

  printf("\033[0;32m请输入要处理通知的序号:>\033[0m");
  fflush(stdout); // 手动刷新标准输出缓冲区

}

void setupmenu(user& u, void *p){
  system("clear");
  fflush(stdout); // 手动刷新标准输出缓冲区
  printf("\033[0;32m数据请求中...\033[0m\n");
  fflush(stdout); // 手动刷新标准输出缓冲区
  system("clear");
  printf("\033[0;36m==========================================================\033[0m\n");
  printf("                \033[0;33m设置与退出\033[0m\n");
  reportfucs::newreport(u, p);
  printf("\033[0;32m[1] 修改密码\033[0m\n");
  printf("\033[0;32m[2] 注销账号\033[0m\n");
  printf("\033[0;32m[3] 查看账户信息\033[0m\n");
  printf("\033[0;32m[4] 退出登录\033[0m\n");
  printf("                    \033[0;33m(tip:ESC可返回上一级菜单)\033[0m\n");
  printf("\033[0;36m==========================================================\033[0m\n");

  printf("\033[0;32m请输入要选择功能的序号:>\033[0m");
}



