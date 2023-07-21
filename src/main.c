#include "common.h"
#include "tokenize.h"
#include "parse.h"
#include "codegen.h"

/* グローバル変数定義 */
char *user_input;

int main(int argc, char **argv){
  DEBUG_WRITE("test\n");
	// 入力データの確認
  if(argc != 2){
    fprintf(stderr, "エラー:引数の個数が正しくありません\n");
    return 1;
  }
	// 抽象構文木作成
	user_input = argv[1];
  LVar_t dummy = {NULL, NULL, 0, 0};
  locals = &dummy;
  // トークナイズ
	token = tokenize(argv[1]);
  exit(1);
  // パーサ
	program();

	// 前半部分のコード生成
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  
  // プロローグ
  // 変数26個分(8byte*26=208byte)の領域を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for(int i=0; code[i] != NULL; i++){
    gen(code[i]);
    // これの必要性は？
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がRAXに残っているのでそれが返り値になる
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}

void debug_write(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}