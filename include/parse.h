#ifndef PARSE_H
#define PARSE_H

#include "tokenize.h"

/// @brief 抽象構文木のノードの種類
typedef enum{
  ND_ADD, 				// +
  ND_SUB, 				// -
  ND_MUL, 				// *
  ND_DIV, 				// /
  ND_ASSIGN,      // =
  ND_LVAR,        // ローカル変数
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
  ND_NUM, 				// 整数
} NodeKind;

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
	int val;			  // kindがND_NUMの場合のみ使う
  LVar_t *lvar;   // ラベル用
};

/* グローバル変数 */
/// @brief 関数のノードの配列
extern Node_t *code[100];

/* 関数プロトタイプ宣言 */
/// @brief パースプログラム
void program(void);

#endif