#include "./SendEmail.h"

size_t EmailSender::payload_source(void* ptr, size_t size, size_t nmemb, void* userp) {
    std::string* payload = (std::string*)userp;
    size_t buffer_size = size * nmemb;
    size_t copy_len = (payload->size() < buffer_size) ? payload->size() : buffer_size;

    if (copy_len > 0) {
        memcpy(ptr, payload->c_str(), copy_len);
        payload->erase(0, copy_len);
        return copy_len;
    }
    return 0;
}

void EmailSender::getcode() {
    std::random_device rd;
    std::mt19937 gen(rd());
    // 生成100000-999999之间的随机数
    std::uniform_int_distribution<> dist(100000, 999999);
    sprintf(code, "%d", dist(gen));
}

bool EmailSender::send(const std::string& receiver_email) {
    getcode();
    std::string to = "<" + receiver_email + ">";

    std::string message =
        "To: " + receiver_email + "\r\n"
        "From: Chatroom<" + sender_user + ">\r\n"
        "Subject: 验证码邮件\r\n"
        "\r\n"
        "您的验证码是：" + std::string(code) + "\r\n";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Curl初始化失败\n";
        return false;
    }

    struct curl_slist* recipients = nullptr;
    recipients = curl_slist_append(recipients, to.c_str());

    curl_easy_setopt(curl, CURLOPT_USERNAME, sender_user.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, sender_pass.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, smtp_server.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, sender_email.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);

    std::string payload = message;
    curl_easy_setopt(curl, CURLOPT_READDATA, &payload);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curl, CURLOPT_PROXY, "");

    CURLcode res = curl_easy_perform(curl);
    bool success = (res == CURLE_OK);

    if (!success) {
        printf("\033[0;31m\n验证码发送失败: %s\n\033[0m", curl_easy_strerror(res));
    } else {
        printf("\033[0;32m\n已发送验证码!\033[0m\n");
        fflush(stdout);  // 手动刷新标准输出缓冲区
    }

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return success;
}