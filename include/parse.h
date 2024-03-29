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
	ND_LESS_THAN,   // <
	ND_LESS_THAN_OR_EQUALT_TO,    // <=
	ND_RETURN,      // return
	ND_IF,          // if
	ND_ELSE,        // else
	ND_SWITCH,			// switch
	ND_CASE,				// case
	ND_DEFAULT,     // default
	ND_WHILE,       // while
	ND_FOR,         // for
	ND_BLOCK,       // block
	ND_FUNCCALL,    // funcCall
	ND_FUNCDEFINE,  // funcDefine
	ND_FUNCDECLARATION, // funcDeclaration
	ND_ADDR,        // アドレス演算子
	ND_DEREF,       // 関節演算子
	ND_NUM, 				// 整数
	ND_STRING, 			// 文字列リテラル
	ND_SINGLESTMT,	// 文
	ND_DOUBLESTMT,	// 複文
	ND_DECLARATION, // 宣言
	ND_STRUCT_MEMBER,
	ND_STRUCT,
} NodeKind;

typedef enum {
	DT_INT,
	DT_CHAR,
	DT_VOID,
	DT_PTR,
	DT_ARRAY,
	DT_STRUCT,
	DT_STRUCT_MEMBER,
	DT_FUNC,
} DataType;

typedef enum {
	IK_LVAR,
	IK_GVAR,
	IK_STRUCT_MEMBER,
	IK_FUNC,
} IdentifierKind;

/*
 * | 種類                   | 名前空間 |
 * |------------------------+----------|
 * | オブジェクト名(変数名) |          |
 * | 関数名                 |  共通の  |
 * | 関数原型名             | 名前空間 |
 * | 列挙子                 |          |
 * | 型定義名               |          |
 * |------------------------+----------| 
 * | 構造体タグ             |  共通の  |
 * | 共用体タグ             | 名前空間 |
 * | 列挙体タグ             |          |
 * |------------------------+----------| 
 * | メンバ                 |タグごとの|
 * |                        | 名前空間 |
 * |------------------------+----------| 
 * | 名札名                 |関数ごとの|
 * |                        | 名前空間 |
 * |------------------------|----------| 
 */

/// @brief ラベルの型
typedef struct Label_tag {
	int label;
	int labelIndex;
} Label_t;

/// @brief 識別子の型
typedef struct Identifier_tag {
	IdentifierKind kind;
	struct Identifier_tag* next;
	struct Identifier_tag* member_list; // typeの基本型がstructの場合のみ使用
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
	Label_t *label; // kindがND_CASEの場合のみ使う
};

typedef struct Types_tag {
	DataType dataType;
	struct Types_tag *inner;
	size_t array_size;
	size_t struct_size;
	char *struct_name;
	int struct_name_len;
} Types_t;


/* グローバル変数 */
/// @brief 関数のノードの配列
extern Node_t *code[CodeSize];
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

Label_t* new_label(int labelVar, int labelIndexVar);

Vector_t* new_vector(Node_t *node, Vector_t *current);

Node_t *new_node(NodeKind kind, Node_t *expr1, Node_t *expr2, Node_t *expr3, Node_t *expr4, Node_t *expr5, Vector_t *vector);

Node_t *new_node_num(int val);

#endif
