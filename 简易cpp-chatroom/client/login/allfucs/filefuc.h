#pragma once
#include "../MessageQueue.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>
#include "../../client.h"
#include "./reportfuc.h"

#include "../../../message.h"
#include "../../../group.h"
#include "../../../user.h"
#include "../../../report.h"

extern std::string server_ip;
extern int server_port;

extern std::atomic<bool> file_sending;
extern std::atomic<bool> file_recving;
extern Client dataclient;
extern FILE* file;
extern char arr[1024];
extern char path[3072];
extern std::string fid;
extern std::string sd;
extern std::string uuid;

extern user us;
extern report rpt;

class filefucs{
private:
    void* clientp;
    int page = 0;
    friendnamelist fnl;

public:

    filefucs(user& arg1, void*p):clientp(p){
    };
    //发送文件线程
    void upload_file_with_offset();
    void upload_gfile_with_offset();
    void download_file_with_offset(std::string);
    void download_gfile_with_offset(std::string);
    //连接上服务器的文件传输线程
    bool conntect_filepth();
    //获取文件名
    std::string GetFileName(const char arr[]);
    //处理发送文件的逻辑
    void sendfile_touser(char, int fg = 1);
    //列好友表
    void list(char, int fg = 1);
    void listfriend();
    //列群聊表
    void listgroup();
    //列文件表
    void filelist(char, int fg = 1);
    //下载好友文件
    std::string formatFileSize(uint64_t size_bytes);
    void downloadfile();
    void downloadgfile();
    void download(char, int fg);
};
