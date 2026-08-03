#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    // 连接到服务器
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "创建socket失败" << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1145);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "连接服务器失败" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "✓ 已连接到服务器" << std::endl;

    // 发送登录请求
    std::string login_msg = "pwlg:aaaa:12345678";
    uint32_t msg_len = htonl(login_msg.size());

    // 先发送长度
    send(sock, &msg_len, sizeof(msg_len), 0);
    // 再发送消息内容
    send(sock, login_msg.c_str(), login_msg.size(), 0);

    std::cout << "✓ 已发送登录请求: " << login_msg << std::endl;

    // 接收响应
    uint32_t recv_len;
    int n = recv(sock, &recv_len, sizeof(recv_len), 0);
    if (n <= 0) {
        std::cerr << "✗ 接收长度失败" << std::endl;
        close(sock);
        return 1;
    }

    recv_len = ntohl(recv_len);
    std::cout << "✓ 收到响应长度: " << recv_len << std::endl;

    char buffer[8192];
    n = recv(sock, buffer, recv_len, 0);
    if (n <= 0) {
        std::cerr << "✗ 接收消息失败" << std::endl;
        close(sock);
        return 1;
    }

    buffer[n] = '\0';
    std::cout << "✓ 收到响应: " << buffer << std::endl;

    close(sock);
    return 0;
}
