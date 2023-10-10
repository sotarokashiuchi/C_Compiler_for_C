#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <string.h>

// ソースコードの行数
#define PROGRAM_LINE 128

/// @brief トークンの種類
typedef enum{
	TK_RESERVED,		// 記号
	TK_IDENT,				// 識別子
	TK_KEYWORD,			// Keyword(予約語)
	TK_NUM,					// 整数トークン
	TK_STRING,			// 文字列リテラル
	TK_CHARACTER,		// 
	TK_EOF,					// 入力の終わりを表すトークン
	TK_ERROR,				// エラートークン
} TokenKind;

/// @brief トークンの型
typedef struct Token {
	TokenKind kind;	// トークンの種類
	struct Token* next;	// 次の入力トークン
	int val;				// kindがTK_NUMの場合、その数値
	char *str;			// トークン文字列
	int	len;				// トークンの長さ
} Token_t;

/* グローバル変数宣言 */
/// @brief 現在着目しているトークン
extern Token_t *token;
/// @brief 入力文字列
extern char *user_input;
// 文字列位置
extern char charPoint [128];


/* プロトタイプ宣言 */
/// @brief 入力文字列pをトークナイズする
/// @param p トークナイズしたい文字列
/// @return トークンリストの先頭アドレス
extern Token_t* tokenize(void);

/// @brief 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
///				 それ以外の場合にはエラーを報告する。
/// @return 次のトークンの数値
extern int expect_number();

/// @brief 次のトークンが期待している記号の場合には、
/// 			 トークンを1つ読み進めて真を返す。それ以外の場合には偽を返す。
/// @param kind 期待する文字列のトークンの種類
/// @param op 期待する記号
/// @return true:次のトークンが期待通り. false:それ以外の場合
extern bool consume(TokenKind kind, char *op);

Token_t* peek(TokenKind kind, char *op);

/// @brief 次のトークンが期待している文字列の場合、トークンを1つ読み進める。
///				 それ以外の場合にはエラーを報告する。
/// @param kind 期待する文字列のトークンの種類
/// @param op 期待する文字列
extern void expect(TokenKind kind, char *op);

/// @brief tokenグローバル変数に任意のトークンを代入する
/// @param tok 任意のトークン
void back_token(Token_t *tok);

Token_t* consume_string(void);
Token_t* consume_character(void);

/// @brief  次のトークンが識別子か調べ、識別子の場合には、
///					トークンを1つ読み進めてトークン構造体を返す。それ以外の場合にはNULLを返す。
/// @return Token_t:トークンが識別子. NULL:トークンが非識別子
Token_t* consume_ident(void);

/// @brief トークンが終端か確認する
/// @return 終端だとtrue, 非終端だとfalseを返す
bool at_eof();

#endif
