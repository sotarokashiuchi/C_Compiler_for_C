#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

/* マクロ定義 */
// ローカル変数用のスタックサイズ 26文字 * 8byte
#define local_variable_stack (26*8)

// デバッグモードの切り替えマクロ
#define DEBUG_WRITE(...) debug_write("[%s:%s:%d]", __FILE__, __func__, __LINE__);debug_write(__VA_ARGS__)
// #define DEBUG_WRITE(...) " "

/* 関数定義 */
/// @brief デバッグ用の情報を表示
/// @param fmt printf()と同様に使用できる
void debug_write(char *fmt, ...);

extern int debugEnabled;

#endif