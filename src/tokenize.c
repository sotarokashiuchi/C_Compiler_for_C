#include <ctype.h>
#include "common.h"
#include "tokenize.h"

/* グローバル変数定義 */
Token_t *token;

/// @brief 新しいトークンを作成し、リストにつなげる
/// @param kind 新しいトークンの種類
/// @param cur リストの末尾トークン
/// @param str 新しいトークンの文字列
/// @return 新しいトークンの先頭アドレス
static Token_t* new_token(TokenKind kind, Token_t *cur, char *str);

/// @brief エラー処理
/// @param loc エラー発生文字
/// @param fmt 出力フォーマット(標準入出力関数と同じ仕様)
static void error_at(char *loc, char *fmt, ...);


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

static Token_t* new_token(TokenKind kind, Token_t *cur, char *str){
	Token_t *new_tok = calloc(1, sizeof(Token_t));
	new_tok->kind = kind;
	new_tok->str = str;
	cur->next = new_tok;
	return new_tok;
}

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


bool consume(char op){
	if(token->kind != TK_RESERVED || token->str[0] != op){
		return false;
	}
	token = token->next;
	return true;
}

void expect(char op) {
	if(token->kind != TK_RESERVED || token->str[0] != op){
		error_at(token->str, "'%c'ではありません", op);
	}
	token = token->next;
}

static void error_at(char *loc, char *fmt, ...){
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

