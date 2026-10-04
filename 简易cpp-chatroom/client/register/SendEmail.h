#pragma once
#include <iostream>
#include <string>
#include <random>
#include <string.h>
#include <cstdlib>
#include <curl/curl.h>

class EmailSender {
private:
    std::string smtp_server;
    std::string sender_email;
    std::string sender_user;
    std::string sender_pass;

    static size_t payload_source(void* ptr, size_t size, size_t nmemb, void* userp);
    void getcode();

public:
    char code[10];  // 用于存储生成的验证码

    EmailSender();
    bool send(const std::string& receiver_email);  // 发送邮件的方法
};
