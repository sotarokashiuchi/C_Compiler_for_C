#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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

/// @brief トークンの種類
typedef enum{
	TK_RESERVED,		// 記号
	TK_NUM,					// 整数トークン
	TK_EOF,					// 入力の終わりを表すトークン
} TokenKind;

/// @brief トークンの型
typedef struct Token {
	TokenKind kind;	// トークンの種類
	struct Token* next;	// 次の入力トークン
	int val;				// kindがTK_NUMの場合、その数値
	char *str;			// トークン文字列
} Token_t;

/* グローバル変数 */
// 現在着目しているトークン
Token_t *token;
char *user_input;


/// @brief エラー処理
/// @param loc エラー発生文字
/// @param fmt 出力フォーマット(標準入出力関数と同じ仕様)
void error_at(char *loc, char *fmt, ...){
	// 可変長引数の処理
	va_list ap;
	va_start(ap, fmt);

	// ポインタ演算(先頭からの文字数計算)
	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

/// @brief 次のトークンが期待している記号の場合には、
/// 			 トークンを1つ読み進めて真を返す。それ以外の場合には偽を返す。
/// @param op 期待する記号
/// @return true:次のトークンが期待通り. false:それ以外の場合
bool consume(char op){
	if(token->kind != TK_RESERVED || token->str[0] != op){
		return false;
	}
	token = token->next;
	return true;
}

/// @brief 次のトークンが期待している記号の場合、トークンを1つ読み進める。
///				 それ以外の場合にはエラーを報告する。
/// @param op 期待する記号
void expect(char op) {
	if(token->kind != TK_RESERVED || token->str[0] != op){
		error_at(token->str, "'%c'ではありません", op);
	}
	token = token->next;
}

/// @brief 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
///				 それ以外の場合にはエラーを報告する。
/// @return 次のトークンの数値
int expect_number() {
	if(token->kind != TK_NUM){
		error_at(token->str, "数値ではありません");
	}
	int val = token->val;
	token = token->next;
	return val;
}

/// @brief 
/// @return 
bool at_eof(){
	return token->kind == TK_EOF;
}

/// @brief 新しいトークンを作成し、リストにつなげる
/// @param kind 新しいトークンの種類
/// @param cur リストの末尾トークン
/// @param str 新しいトークンの文字列
/// @return 新しいトークンの先頭アドレス
Token_t* new_token(TokenKind kind, Token_t *cur, char *str){
	Token_t *new_tok = calloc(1, sizeof(Token_t));
	new_tok->kind = kind;
	new_tok->str = str;
	cur->next = new_tok;
	return new_tok;
}

/// @brief 入力文字列pをトークナイズする
/// @param p トークナイズしたい文字列
/// @return トークンリストの先頭アドレス
Token_t* tokenize(char *p){
	// ダミーリスト(トークンリストの先頭)
	Token_t head;
	head.next = NULL;
	// トークンリスト
	Token_t *cur = &head;

	while(*p){
		// 空白文字をスキップ
		if(isspace(*p)){
			p++;
			continue;
		}

		// 記号処理
		if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'){
			cur = new_token(TK_RESERVED, cur, p);
			p++;
			continue;
		}

		// 数値処理
		if(isdigit(*p)){
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}
		
		error_at(token->str, "トークナイズできません");
	}

	// トークンリストの末尾を作成
	new_token(TK_EOF, cur, p);
	return head.next;
}

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