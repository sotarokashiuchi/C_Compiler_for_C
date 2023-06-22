#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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

// 現在着目しているトークン
Token_t *token;
/// @brief エラー処理
/// @param fmt 出力フォーマット(標準入出力関数と同じ仕様)
void error(char *fmt, ...){
	// 可変長引数の処理
	va_list ap;
	va_start(ap, fmt);

	// エラー文の表示
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
		error("'%c'ではありません", op);
	}
	token = token->next;
}

/// @brief 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
///				 それ以外の場合にはエラーを報告する。
/// @return 次のトークンの数値
int expect_number() {
	if(token->kind != TK_NUM){
		error("数値ではありません");
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
		if(*p == '+' || *p == '-'){
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
		
		error("トークナイズできません");
	}

	// トークンリストの末尾を作成
	new_token(TK_EOF, cur, p);
	return head.next;
}

int main(int argc, char **argv){
	// 入力データの確認
  if(argc != 2){
    fprintf(stderr, "エラー:引数の個数が正しくありません\n");
    return 1;
  }

	// 前半部分のコード生成
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

	// トークナイズする
	token = tokenize(argv[1]);

	// 式の初めは必ず数値のため、それをチェックしコード生成
  printf("  mov rax, %d\n", expect_number());

	while(!at_eof()){
		if(consume('+')){
			printf("\tadd rax, %d\n", expect_number());
			continue;
		}

		expect('-');
		printf("\tsub rax, %d\n", expect_number());
	}

  printf("  ret\n");
  return 0;
}