#ifndef PARSE_H
#define PARSE_H

#include "tokenize.h"

/// @brief 抽象構文木のノードの種類
typedef enum{
  ND_ADD, 				// +
  ND_SUB, 				// -
  ND_MUL, 				// *
  ND_DIV, 				// /
  ND_MOD,         // %
  ND_LOGICAL_AND,	// &&
  ND_LOGICAL_OR,	// ||
  ND_ASSIGN_EQ,   // =
  ND_ASSIGN_ADD,  // +=
  ND_ASSIGN_SUB,  // -=
  ND_ASSIGN_MUL,  // *=
  ND_ASSIGN_DIV,  // /=
  ND_ASSIGN_MOD,  // %=
  ND_LVAR,        // ローカル変数
  ND_GVAR,        // グローバル変数
  ND_GVARDEFINE,  // グローバル変数定義
  ND_EQUALTO,     // ==
  ND_NOT_EQUAL_TO,// !=
  ND_GREATER_THAN,// >
  ND_LESS_THAN,   // <
  ND_GREATER_THAN_OR_EQUAL_TO,  // >=
  ND_LESS_THAN_OR_EQUALT_TO,    // <=
  ND_RETURN,      // return
  ND_IF,          // if
  ND_ELSE,        // else
  ND_WHILE,       // while
  ND_FOR,         // for
  ND_BLOCK,       // block
  ND_FUNCCALL,    // funcCall
  ND_FUNCDEFINE,  // funcDefine
  ND_ADDR,        // アドレス演算子
  ND_DEREF,       // 関節演算子
  ND_NUM, 				// 整数
	ND_STRING, 			// 文字列リテラル
	ND_SINGLESTMT,	// 文
	ND_DOUBLESTMT,	// 複文
} NodeKind;

typedef enum {
  DT_INT,
	DT_CHAR,
  DT_PTR,
  DT_ARRAY,
  DT_FUNC,
} DataType;

typedef enum {
	IK_LVAR,
	IK_GVAR,
	IK_FUNC,
} IdentifierKind;

/// @brief 識別子の型
typedef struct Identifier_tag {
	IdentifierKind kind;
	struct Identifier_tag* next;
	char *name;
	int len;
	int offset;
  struct Types_tag *type;
} Identifier_t;

/// @brief 文字列リテラルのベクタリスト
typedef struct StringVector_tag {
	char *string;
	int len;
	int labelID;
	struct StringVector_tag *next;
} StringVector_t;

typedef struct Node_tag Node_t;
typedef struct Vector_tag Vector_t;

/// @brief ベクタのリスト
struct Vector_tag {
  Node_t *node;   // 単文のポインタ
  Vector_t *prev; // 前のVector_t
  Vector_t *next; // 次のVector_t
};

/// @brief 抽象構文木のノードの型
struct Node_tag {
	NodeKind kind;	// ノードの型
	Node_t *expr1;	// Node1
	Node_t *expr2;  // Node2
  Node_t *expr3;  // Node3
  Node_t *expr4;  // Node4
  Node_t *expr5;  // Node5
  Vector_t *vector;
  struct Types_tag *type;
	int val;			  // kindがND_NUMの場合のみ使う
  Identifier_t *identifier;   // 識別子用
	StringVector_t *string;
};

typedef struct Types_tag {
  DataType dataType;
  struct Types_tag *inner;
  size_t array_size;
} Types_t;


/* グローバル変数 */
/// @brief 関数のノードの配列
extern Node_t *code[100];
/// @brief Lvar_t構造体のリストの先頭
extern Identifier_t *identHead;
/// @brief Lvar_t構造体のリストの先頭
extern StringVector_t *stringHead;

/* 関数プロトタイプ宣言 */
/// @brief パースプログラム
void program(void);
/// @brief 変数名を検索する
/// @param tok 検索したい変数の情報が格納されたLVar_t
/// @return 一致したLVar_tを返す, 一致しなければNULLを返す
Identifier_t* find_lvar(Token_t *tok);

/// @brief 型のsizeを返す
/// @param type 
/// @return 
int sizeofType(Types_t *type);

#endif
