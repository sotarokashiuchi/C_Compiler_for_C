#include <ctype.h>
#include "common.h"
#include "tokenize.h"

/* グローバル変数定義 */
// 現在のトークン
Token_t *token;
// 文字列位置
char charPoint[PROGRAM_LINE];

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
static void error_at(Token_t *loc, char *fmt, ...);

/// @brief トークンを構成する文字か判断
/// @param c 判断したい文字
/// @return 1:トークンを構成する文字列, 0:トークンを構成しない文字列
int is_alnum(char c);

Token_t* tokenize(void){
	char *p=NULL;
	int count, line;
	char *row;
	// ダミーリスト(トークンリストの先頭)
	Token_t head;
	head.next = NULL;
	// トークンリスト
	Token_t *cur = &head;

	for(row = p = user_input, line=1; *p; ){
		// 改行
		if(*p == '\n'){
			// '\n'も1文字と数える
			charPoint[line] = (p-row) + 1;
			p++;
			row=p;
			line++;
			continue;
		}
		p++;
	}

	for(count=0, p=user_input; *p; count++){
		// 制御文字、空白文字をスキップ
		if(isspace(*p)){
			p++;
			count--;
			continue;
		}

		// keyword
		if(!strncmp(p, "return", 6) && !is_alnum(p[6])){
			cur = new_token(TK_KEYWORD, cur, &p, 6);
			continue;
		}

		if(!strncmp(p, "int", 3) && !is_alnum(p[3])){
			cur = new_token(TK_KEYWORD, cur, &p, 3);
			continue;
		}

		if(!strncmp(p, "if", 2) && !is_alnum(p[2])){
			cur = new_token(TK_KEYWORD, cur, &p, 2);
			continue;
		}

		if(!strncmp(p, "else", 4) && !is_alnum(p[4])){
			cur = new_token(TK_KEYWORD, cur, &p, 4);
			continue;
		}

		if(!strncmp(p, "while", 5) && !is_alnum(p[5])){
			cur = new_token(TK_KEYWORD, cur, &p, 5);
			continue;
		}

		if(!strncmp(p, "for", 3) && !is_alnum(p[3])){
			cur = new_token(TK_KEYWORD, cur, &p, 3);
			continue;
		}

		// 2文字
		if(!strncmp(p, "<=", 2) || !strncmp(p, ">=", 2) || !strncmp(p, "==", 2) || !strncmp(p, "!=", 2)){
			cur = new_token(TK_RESERVED, cur, &p, 2);
			continue;
		}

		// 1文字
		if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=' || *p == '{' || *p == '}' || *p == ',' || *p == '&'){
			cur = new_token(TK_RESERVED, cur, &p, 1);
			continue;
		}
		
		// 変数(複数文字列)
		if(isalpha(*p) || *p == '_'){
			int i;
			char *tmpp = p;
			tmpp++;
			// 英数字又は_である
			for(i=1; isalnum(*tmpp) || *tmpp == '_'; i++){
				tmpp++;
			}
			cur = new_token(TK_IDENT, cur, &p, i);
			continue;
		}

		// 数値処理
		if(isdigit(*p)){
			cur = new_token(TK_NUM, cur, &p, 0);
			cur->val = strtol(p, &p, 10);
			continue;
		}
		
		cur = new_token(TK_ERROR, cur, &p, 1);
		error_at(cur, "トークナイズできません");
	}

	// トークンリストの末尾を作成
	new_token(TK_EOF, cur, &p, 0);
	DEBUG_WRITE("number of token is %d\n", count);
	return head.next;
}

static Token_t* new_token(TokenKind kind, Token_t *cur, char **str, int len){
	DEBUG_WRITE(": kind of token is %d (str:\"%.10s...\" len:%d)\n", kind, *str, len);
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
		error_at(token, "数値ではありません");
	}
	int val = token->val;
	token = token->next;
	return val;
}

int is_alnum(char c){
	  return 	('a' <= c && c <= 'z') ||
						('A' <= c && c <= 'Z') ||
						('0' <= c && c <= '9') ||
						(c == '_');
}

bool at_eof(){
	return token->kind == TK_EOF;
}


bool consume(TokenKind kind, char *op){
	if(token->kind != kind || token->len != strlen(op) || memcmp(token->str, op, token->len)){
		return false;
	}
	token = token->next;
	return true;
}

bool peek(TokenKind kind, char *op){
	if(token->kind != kind || token->len != strlen(op) || memcmp(token->str, op, token->len)){
		return false;
	}
	return true;
}

void expect(TokenKind kind, char *op) {
	if(token->kind != kind || token->len != strlen(op) || memcmp(token->str, op, token->len)){
		error_at(token, "'%s'ではありません", op);
	}
	token = token->next;
}

void back_token(Token_t *tok){
	token = tok;
	return;
}

Token_t* consume_ident(void){
	if(token->kind != TK_IDENT){
		return NULL;
	}
	Token_t *tok = token;
	token = token->next;
	return tok;
}

static void error_at(Token_t *loc, char *fmt, ...){
	// 可変長引数の処理
	va_list ap;
	va_start(ap, fmt);
	int row, line;
	char *outputLine;
	int errorPoint = loc->str - user_input + 1;
	int charTotal;
	
	for(line=1, charTotal=0; line<PROGRAM_LINE; line++){
		charTotal += charPoint[line];
		if(errorPoint<=charTotal){
			// 列計算
			row = errorPoint - (charTotal - charPoint[line]);
			// エラー行の末尾にヌル文字を追加
			outputLine = user_input+charTotal-1;
			*outputLine = '\0';
			outputLine = user_input+charTotal-charPoint[line];
			break;
		}
	}
	fprintf(stderr, "%d:%d \n", line, row);
	fprintf(stderr, "%8d |  %s\n", line, outputLine);
	fprintf(stderr, "         |  \x1b[31m%*s-\x1b[0m", row, "^");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

