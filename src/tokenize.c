#include <ctype.h>
#include "common.h"
#include "tokenize.h"

/* グローバル変数定義 */
Token_t *token;

/// @brief 新しいトークンを作成し、リストにつなげる
/// @param kind 新しいトークンの種類
/// @param cur リストの末尾トークン
/// @param str 新しいトークンの文字列のアドレスを格納する変数のアドレス
/// @param len トークン文字列の長さ
/// @return 新しいトークンの先頭アドレス
static Token_t* new_token(TokenKind kind, Token_t *cur, char **str, int len);

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

		// 2文字
		if(!strncmp(p, "<=", 2) || !strncmp(p, ">=", 2) || !strncmp(p, "==", 2) || !strncmp(p, "!=", 2)){
			cur = new_token(TK_RESERVED, cur, &p, 2);
			continue;
		}

		// 1文字
		if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '='){
			cur = new_token(TK_RESERVED, cur, &p, 1);
			continue;
		}
		
		// 変数
		if('a' <= *p && *p <= 'z'){
			cur = new_token(TK_IDENT, cur, &p, 1);
			continue;
		}

		// 数値処理
		if(isdigit(*p)){
			cur = new_token(TK_NUM, cur, &p, 0);
			cur->val = strtol(p, &p, 10);
			continue;
		}
		
		error_at(cur->str, "トークナイズできません");
	}

	// トークンリストの末尾を作成
	new_token(TK_EOF, cur, &p, 0);
	return head.next;
}

static Token_t* new_token(TokenKind kind, Token_t *cur, char **str, int len){
	Token_t *new_tok = calloc(1, sizeof(Token_t));
	new_tok->kind = kind;
	new_tok->str = *str;
	new_tok->len = len;
	cur->next = new_tok;
	
	// token文字列を読み進める
	while(len--){
		(*str)++;
	}

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

bool at_eof(){
	return token->kind == TK_EOF;
}


bool consume(char *op){
	if(token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len)){
		return false;
	}
	token = token->next;
	return true;
}

void expect(char *op) {
	if(token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len)){
		error_at(token->str, "'%c'ではありません", op);
	}
	token = token->next;
}

Token_t* consume_ident(void){
	if(token->kind != TK_IDENT){
		return NULL;
	}
	Token_t *tok = token;
	token = token->next;
	return tok;
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

