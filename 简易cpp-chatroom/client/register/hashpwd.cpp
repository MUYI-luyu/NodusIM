#include "hashpwd.h"

// 构造函数：初始化 libsodium 库
PasswordHasher::PasswordHasher() {
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium 初始化失败");
    }
}

// 生成密码哈希，覆盖原密码内容（原地修改）
void PasswordHasher::hashPassword(char* passwd) {
    // 先保存原密码
    std::string orig(passwd);

    // 生成哈希，覆盖原内存
    if (crypto_pwhash_str(
            passwd,
            orig.c_str(),
            orig.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("生成密码哈希失败");
    }
}

// 验证密码是否和哈希匹配
bool PasswordHasher::verifyPassword(const char* passwd, const char* hashed) const {
    return crypto_pwhash_str_verify(hashed, passwd, std::strlen(passwd)) == 0;
}
