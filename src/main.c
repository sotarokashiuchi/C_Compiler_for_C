#include "common.h"
#include "tokenize.h"
#include "parse.h"
#include "codegen.h"

/* グローバル変数定義 */
char *user_input;

int main(int argc, char **argv){
	// 入力データの確認
  if(argc != 2){
    fprintf(stderr, "エラー:引数の個数が正しくありません\n");
    return 1;
  }
	// 抽象構文木作成
	user_input = argv[1];
	token = tokenize(argv[1]);
	Node_t *node = expr();
  exit(1);

	// 前半部分のコード生成
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

	gen(node);

  // 計算結果をRAXに移動
	printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}