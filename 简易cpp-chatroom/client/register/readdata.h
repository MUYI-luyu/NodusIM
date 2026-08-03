#pragma once
#include <iostream>
#include <string>

#define KEY_ESC     1000
#define KEY_UP      1001
#define KEY_DOWN    1002
#define KEY_LEFT    1003
#define KEY_RIGHT   1004

// 函数声明
char charget(void);
int tm_charget(int timeout_ms);
int sptm_charget(int timeout_ms, char *utf8);
std::string tm_read_all_input(int timeout_ms);
int utf8_char_length(unsigned char c);
int is_wide_char(const unsigned char *p);
int enter(char *arr, int key);
template <size_t N>
void chu(char (&arr)[N]) {
    memset(arr, 0, N);
}
int is_valid_utf8_char(const unsigned char *p, int len);
int is_valid_username(const char *name);
