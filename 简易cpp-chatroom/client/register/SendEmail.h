#pragma once
#include <iostream>
#include <string>
#include <random>
#include <string.h>
#include <curl/curl.h>

class EmailSender {
private:
    std::string smtp_server = "smtps://smtp.qq.com:465";
    std::string sender_email = "<3299956263@qq.com>";
    std::string sender_user = "3299956263@qq.com";
    std::string sender_pass = "vqcxmenfclxjcjgf";

    static size_t payload_source(void* ptr, size_t size, size_t nmemb, void* userp);
    void getcode();

public:
    char code[10];  // 用于存储生成的验证码

    // 构造函数不需要显式定义，使用默认构造函数
    bool send(const std::string& receiver_email);  // 发送邮件的方法
};
