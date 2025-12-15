#pragma once
#include <cstdio>

// 使用颜色

#define ERROR(fmt, ...) \
printf("\033[1;31m[ERROR] %s:%d: " fmt "\033[0m\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define WARNING(fmt, ...) \
printf("\033[1;33m[WARNING] %s:%d: " fmt "\033[0m\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define INFO(fmt, ...) \
printf("\033[1;32m[INFO] %s:%d: " fmt "\033[0m\n", __FILE__, __LINE__, ##__VA_ARGS__)
