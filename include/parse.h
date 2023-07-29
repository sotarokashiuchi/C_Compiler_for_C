#ifndef PARSE_H
#define PARSE_H

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
  ND_IF,
  ND_ELSE,
  ND_WHILE,
  ND_NUM, 				// 整数
} NodeKind;

/// @brief 抽象構文木のノードの型
typedef struct Node {
	NodeKind kind;		// ノードの型
	struct Node *expr1;	// 左辺
	struct Node *expr2; // 右辺
  struct Node *expr3;
  struct Node *expr4;
  struct Node *expr5;
	int val;					// kindがND_NUMの場合のみ使う
	int offset;			  // kindがND_LVAEの場合のみ使う
} Node_t;

// グローバル変数
extern Node_t *code[100];

void program(void);

#endif