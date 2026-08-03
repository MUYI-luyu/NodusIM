#include "readdata.h"
#include <cstring>
#include <termios.h>  
#include <unistd.h>   

char charget(void){
  struct termios oldt, newt;
  char ch;
  // 获取当前终端配置
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  // 设置终端为原始模式：无回显、无需回车
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  // 读取单个字符
  ch = getchar();
  // 恢复终端配置
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
}

// 等待用户输入一个字符，最多等待 timeout_ms 毫秒
// 成功读取返回字符值，超时返回 -1
int tm_charget(int timeout_ms) {
    struct termios oldt, newt;
    int ch;

    // 获取当前终端配置
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    // 设置非规范模式，无回显
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // 设置select等待输入的时间
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int retval = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
    if (retval == -1) {
        // select 出错
        ch = -1;
    } else if (retval) {
        // 有输入可读
        ch = getchar();
    } else {
        // 超时
        ch = -1;
    }
    // 恢复原来的终端配置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}


int sptm_charget(int timeout_ms, char *utf8) {
    struct termios oldt, newt;
    int ch = -1;

    // 保存当前终端配置
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // 非规范模式，无回显
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // select 等待输入
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int retval = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
    if (retval > 0) {
        unsigned char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 27) { // ESC
                unsigned char seq[2] = {0};
                struct timeval tv2 = {0, 1000}; // 1ms
                fd_set rfds2;
                FD_ZERO(&rfds2);
                FD_SET(STDIN_FILENO, &rfds2);
                if (select(STDIN_FILENO + 1, &rfds2, NULL, NULL, &tv2) > 0) {
                    read(STDIN_FILENO, seq, 2);
                    if (seq[0] == '[') {
                        switch (seq[1]) {
                            case 'A': ch = KEY_UP; break;
                            case 'B': ch = KEY_DOWN; break;
                            case 'C': ch = KEY_RIGHT; break;
                            case 'D': ch = KEY_LEFT; break;
                            default:  ch = KEY_ESC; break;
                        }
                    } else {
                        ch = KEY_ESC;
                    }
                } else {
                    ch = KEY_ESC;
                }
            } else {
                // 普通字符（支持 UTF-8）
                utf8[0] = c;
                int extra = 0;
                if ((c & 0xE0) == 0xC0) extra = 1; // 2字节
                else if ((c & 0xF0) == 0xE0) extra = 2; // 3字节
                else if ((c & 0xF8) == 0xF0) extra = 3; // 4字节

                if (extra > 0) {
                    read(STDIN_FILENO, utf8 + 1, extra);
                }
                utf8[extra + 1] = '\0';
                ch = extra + 1; // 返回 UTF-8 长度
            }
        }
    } else if (retval == -1) {
        ch = -1; // select 出错
    } else {
        ch = -1; // 超时
    }

    // 恢复终端配置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

std::string tm_read_all_input(int timeout_ms) {
    struct termios oldt, newt;
    std::string result;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int retval = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
    if (retval > 0) {
        char buf[32];  // 最多读取32字节，UTF-8输入一次不会超过
        int n = read(STDIN_FILENO, buf, sizeof(buf));
        if (n > 0) result.append(buf, n);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return result;
}

// 判断 UTF-8 首字节，获取字符字节长度
int utf8_char_length(unsigned char c) {
    if ((c & 0x80) == 0x00) return 1;
    else if ((c & 0xE0) == 0xC0) return 2;
    else if ((c & 0xF0) == 0xE0) return 3;
    else if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

// 判断是否为宽字符（中文等），粗略判断 3字节及以上为宽字符
int is_wide_char(const unsigned char *p) {
    int len = utf8_char_length(p[0]);
    return len >= 3;
}

//key为0显式输入，为1隐式输入,为2则为聊天输入（最多500字）,返回读取字符的长度，返回-1说明用户需要ESC
int enter(char *arr, int key) {
    int i = 0;
    unsigned char ch;

    while (1) {
        if(key != 2)
            ch = charget();
        else if(key == 2) {
            ch = tm_charget(200);
            if(ch == -1) return -5;
        }
        if(key != 2 && i >= 25 && ch != '\b' && ch != 127 && ch != '\n' && ch != 27) continue;
        if(key == 2 && i >= 300 && ch != '\b' && ch != 127 && ch != '\n' && ch != 27 && ch != '[' && ch != ']') continue;
        if (ch == 27) return -1;  // ESC
        if (ch == '[') return -2;
        if (ch == ']') return -3;
        if (ch == '\n') {
            arr[i] = '\0';
            break;
        }
        // 回退：退格或DEL
        if (ch == '\b' || ch == 127) {
            if (i == 0) continue;

            // 向前找 UTF-8 起始字节
            int len = 1;
            while (i - len > 0 && (arr[i - len] & 0xC0) == 0x80)
                len++;

            int char_start = i - len;

            int display_width = is_wide_char((unsigned char *)&arr[char_start]) ? 2 : 1;

            // 删除字符
            i = char_start;
            arr[i] = '\0';

            // 清除终端字符（显示宽度）
            for (int j = 0; j < display_width; j++) {
                printf("\b \b");
            }

            fflush(stdout);
            continue;
        }

        // 正常字符输入
        int len = utf8_char_length(ch);
        if (i + len >= 255) continue;

        arr[i++] = ch;
        for (int j = 1; j < len; ++j) {
            unsigned char next = charget();
            arr[i++] = next;
        }
        arr[i] = '\0';

        if (key == 1) {
            int display_width = is_wide_char((unsigned char *)&arr[i - len]) ? 2 : 1;
            for (int j = 0; j < display_width; j++) putchar('*');
        } else {
            fwrite(&arr[i - len], 1, len, stdout);
        }

        fflush(stdout);
    }

    printf("\n");
    return i;
}

// 判断一个 UTF-8 字符是否为英文字母、数字或中文（常见汉字范围）
int is_valid_utf8_char(const unsigned char *p, int len) {
    if (len == 1) {
        return ((p[0] >= 'a' && p[0] <= 'z') ||
                (p[0] >= 'A' && p[0] <= 'Z') ||
                (p[0] >= '0' && p[0] <= '9'));
    } else if (len == 3) {
        unsigned char c1 = p[0];
        // 常见汉字 0xE4~0xE9 开头（UTF-8）
        if (c1 >= 0xE4 && c1 <= 0xE9) return 1;
        return 0;
    }

    // 不允许 emoji、日文、俄文等
    return 0;
}

//用户名判断函数，返回实际字符数，若非法则返回-1
int is_valid_username(const char *name) {
    int i = 0;
    int char_count = 0;

    while (name[i] != '\0') {
        int len = utf8_char_length((unsigned char)name[i]);

        if (name[i + len - 1] == '\0') {
            // 字符末尾不完整（UTF-8 截断）
            return -1;
        }

        if (!is_valid_utf8_char((const unsigned char *)&name[i], len)) {
            return -1;
        }

        i += len;
        char_count++;
    }

    return char_count;
}
