#ifndef PARSE_H
#define PAESE_H

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

/// @brief expr = mul ("+" mul | "-" mul)*
/// @return 生成したノード
extern Node_t *expr(void);

/// @brief mul = primary ("*" primary | "/" primary)*
/// @return 生成したノード
extern Node_t *mul(void);

/// @brief primary = "(" expr ")" | num
/// @return 生成したノード
extern Node_t *primary();

#endif