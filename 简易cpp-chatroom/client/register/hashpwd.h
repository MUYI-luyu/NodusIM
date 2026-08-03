#pragma once
#include <sodium.h>
#include <cstring>
#include <iostream>
#include <termios.h>

class PasswordHasher {
public:
    // 构造函数：初始化 libsodium 库
    PasswordHasher();

    // 生成密码哈希，覆盖原密码内容（原地修改）
    // 输入：char* passwd，必须保证缓冲区至少 crypto_pwhash_STRBYTES 字节大小
    void hashPassword(char* passwd);

    // 验证密码是否和哈希匹配
    // passwd: 明文密码
    // hashed: 哈希字符串
    // 匹配返回1(true)，不匹配返回0(false)
    bool verifyPassword(const char* passwd, const char* hashed) const;
};
