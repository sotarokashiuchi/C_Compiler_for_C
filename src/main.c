// #include <ctype.h>
// #include <stdlib.h>
// #include <string.h>
#include "common.h"
#include "tokenize.h"

/* グローバル変数定義 */
char *user_input;

/* EBNF
 * expr    = mul ("+" mul | "-" mul)*
 * mul     = primary ("*" primary | "/" primary)*
 * primary = num | "(" expr ")"
 */

/// @brief 抽象構文木のノードの種類
typedef enum{
  ND_ADD, 				// +
  ND_SUB, 				// -
  ND_MUL, 				// *
  ND_DIV, 				// /
  ND_NUM, 				// 整数
} NodeKind;

/// @brief 抽象構文木のノードの型
typedef struct Node {
	NodeKind kind;		// ノードの型
	struct Node *lhs;	// 左辺
	struct Node *rhs; // 右辺
	int val;					// kindがND_NUMの場合のみ使う
} Node_t;

/// @brief ノードを生成する
/// @param kind ノードの種類
/// @param lhs 左辺
/// @param rhs 右辺
/// @return 生成したノード
Node_t *new_node(NodeKind kind, Node_t *lhs, Node_t *rhs){
	Node_t *node = calloc(1, sizeof(Node_t));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}


/// @brief numノード(終端記号)を生成する
/// @param val ノードに設定する数値
/// @return 生成したノード
Node_t *new_node_num(int val){
	Node_t *node = calloc(1, sizeof(Node_t));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node_t *expr(void);
/// @brief primary = "(" expr ")" | num
/// @return 生成したノード
Node_t *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume('(')) {
    Node_t *node = expr();
    expect(')');
    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}

/// @brief mul = primary ("*" primary | "/" primary)*
/// @return 生成したノード
Node_t *mul(void){
	Node_t *node = primary();
	
	for(;;){
		if(consume('*')){
			node = new_node(ND_MUL, node, primary());
		} else if(consume('/')){
			node = new_node(ND_DIV, node, primary());
		} else{
			return node;
		}
	}
}

/// @brief expr = mul ("+" mul | "-" mul)*
/// @return 生成したノード
Node_t *expr(void) {
	Node_t *node = mul();

	for(;;){
		if(consume('+')){
			node = new_node(ND_ADD, node, mul());
		} else if(consume('-')){
			node = new_node(ND_SUB, node, mul());
		} else{
			return node;
		}
	}
}

/// @brief 抽象構文木からアセンブリコード生成
/// @param node 抽象構文木の先頭ノード
void gen(Node_t *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv){
	// 入力データの確認
  if(argc != 2){
    fprintf(stderr, "エラー:引数の個数が正しくありません\n");
    return 1;
  }
	// 抽象構文木作成
	user_input = argv[1];
	token = tokenize(user_input);
	Node_t *node = expr();

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