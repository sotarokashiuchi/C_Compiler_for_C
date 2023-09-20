#include "common.h"
#include "tokenize.h"
#include "parse.h"
#include "codegen.h"

/* グローバル変数定義 */
char *user_input;
int debugEnabled;

int main(int argc, char **argv){
  DEBUG_WRITE("\n");
	// 入力データの確認
  if(argc != 2){
    fprintf(stderr, "エラー:引数の個数が正しくありません\n");
    return 1;
  }

  /* 初期化 */
  char *debug = getenv("CC_DEBUG");
  debugEnabled = debug && !strcmp(debug, "1");
	user_input = argv[1];
  Identifier_t dummyIdentifier = {0, NULL, NULL, 0, 0};
  identHead = &dummyIdentifier;
	StringVector_t dummyStringVecotr = {NULL, 0, 0, NULL};
  stringHead = &dummyStringVecotr;

  /* トークナイズ */
  token = tokenize();
	DEBUG_WRITE("\033[35mcompleted tokenize\033[39m\n\n");

  /* パーサ */
	program();
	DEBUG_WRITE("\033[35mcompleted parse\033[39m\n\n");

  /* コード生成 */
  // 前半部分のコード生成
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  for(int i=0; code[i] != NULL; i++){
    gen(code[i]);
  }
	DEBUG_WRITE("\033[35mcompleted codegen\033[39m\n\n");

  return 0;
}

void debug_write(char *fmt, ...){
  if(!debugEnabled){
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}
