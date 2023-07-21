#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
// 可変引数用
#include <stdarg.h>

/// @brief デバッグ用の情報を表示
/// @param fmt printf()と同様に使用できる
void debug_write(char *fmt, ...);
#define DEBUG_WRITE(...) debug_write(__VA_ARGS__)

#endif