#include "common.h"
#include "parse.h"
#include "tokenize.h"

/* EBNF
 * expr    = mul ("+" mul | "-" mul)*
 * mul     = primary ("*" primary | "/" primary)*
 * primary = num | "(" expr ")"
 */

/// @brief ノードを生成する
/// @param kind ノードの種類
/// @param lhs 左辺
/// @param rhs 右辺
/// @return 生成したノード
Node_t *new_node(NodeKind kind, Node_t *lhs, Node_t *rhs){
	Node_t *node = calloc(1, sizeof(Node_t));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}


/// @brief numノード(終端記号)を生成する
/// @param val ノードに設定する数値
/// @return 生成したノード
Node_t *new_node_num(int val){
	Node_t *node = calloc(1, sizeof(Node_t));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node_t *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume('(')) {
    Node_t *node = expr();
    expect(')');
    return node;
  }

  // そうでなければ数値のはず
  return new_node_num(expect_number());
}


Node_t *expr(void) {
	Node_t *node = mul();

	for(;;){
		if(consume('+')){
			node = new_node(ND_ADD, node, mul());
		} else if(consume('-')){
			node = new_node(ND_SUB, node, mul());
		} else{
			return node;
		}
	}
}

Node_t *mul(void){
	Node_t *node = primary();
	
	for(;;){
		if(consume('*')){
			node = new_node(ND_MUL, node, primary());
		} else if(consume('/')){
			node = new_node(ND_DIV, node, primary());
		} else{
			return node;
		}
	}
}