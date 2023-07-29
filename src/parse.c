#include "common.h"
#include "parse.h"
#include "tokenize.h"

/* EBNF
 * program    = stmt* 
 * stmt    = expr ";"
					| "return" expr ";"
					| "if" "(" expr ")" stmt ("else" stmt)?
					| "while" "(" expr ")" stmt
					| "for" "(" expr? ";" expr? ";" expr? ")" stmt
 * expr       = assign
 * assign     = equality ("=" assign)?
 * equality   = relational ("==" relational | "!=" relational)*
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 * add        = mul ("+" mul | "-" mul)*
 * mul        = unary ("*" unary | "/" unary)*
 * unary      = ("+" | "-")? primary
 * primary    = num | ident | "(" expr ")"
 */

// 文ごとの先頭ノードを格納
Node_t *code[100];

// void program(void);
Node_t* stmt(void);
Node_t* expr(void);
Node_t* assign(void);
Node_t* equality(void);
Node_t* relational(void);
Node_t* add(void);
Node_t* mul(void);
Node_t* unary();
Node_t* primary();

/// @brief ノードを生成する
/// @param kind ノードの種類
/// @param expr1 左辺
/// @param expr2 右辺
/// @param expr3 右辺
/// @param expr4 右辺
/// @param expr5 右辺
/// @return 生成したノード
Node_t *new_node(NodeKind kind, Node_t *expr1, Node_t *expr2, Node_t *expr3, Node_t *expr4, Node_t *expr5){
	Node_t *node = calloc(1, sizeof(Node_t));
	node->kind = kind;
	node->expr1 = expr1;
	node->expr2 = expr2;
	node->expr3 = expr3;
	node->expr4 = expr4;
	node->expr5 = expr5;
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

// program    = stmt*
void program(void){
  DEBUG_WRITE("header node\n");
	int i = 0;
	while(!at_eof()){
		code[i++] = stmt();
	}
	code[i] = NULL;
}

//  stmt    = expr ";"
//  				| expr ";"
// 					| "return" expr ";"
// 					| "if" "(" expr ")" stmt ("else" stmt)?
// 					| "while" "(" expr ")" stmt
// 					| "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node_t* stmt(void){
  DEBUG_WRITE("\n");
	Node_t *node;
	Node_t *expr1, *expr2, *expr3, *expr4;
	
  if (consume(TK_KEYWORD, "return")) {
		// "return" expr ";"
		node = new_node(ND_RETURN, NULL, expr(), NULL, NULL, NULL);
		expect(TK_RESERVED, ";");
  } else if(consume(TK_KEYWORD, "if")) {
		// "if" "(" expr ")" stmt
		// 左ノードに条件式を 右ノードに処理を
		expect(TK_RESERVED, "(");
		expr1 = expr();
		expect(TK_RESERVED, ")");
		expr2 = stmt();
		node = new_node(ND_IF, expr1, expr2, NULL, NULL, NULL);

		if(consume(TK_KEYWORD, "else")){
			// ("else" stmt)?
			// 左ノードにifを 右ノードに処理を
			node = new_node(ND_ELSE, node, stmt(), NULL, NULL, NULL);
		}
	} else if(consume(TK_KEYWORD, "while")){
		// "while" "(" expr ")" stmt
		expect(TK_RESERVED, "(");
		expr1 = expr();
		expect(TK_RESERVED, ")");
		expr2 = stmt();
		node = new_node(ND_WHILE, expr1, expr2, NULL, NULL, NULL);
	} else if(consume(TK_KEYWORD, "for")){
		// "for" "(" expr? ";" expr? ";" expr? ")" stmt
		// for(A; B; C)D
		expect(TK_RESERVED, "(");
		if(consume(TK_RESERVED, ";")){
			expr1 = NULL;
		}else{
			// Aの読み込み
			expr1 = expr();
			expect(TK_RESERVED, ";");
		}

		if(consume(TK_RESERVED, ";")){
			expr2 = NULL;
		}else{
			// Bの読み込み
			expr2 = expr();
			expect(TK_RESERVED, ";");
		}

		if(consume(TK_RESERVED, ")")){
			expr3 = NULL;
		}else{
			// Cの読み込み
			expr3 = expr();
			expect(TK_RESERVED, ")");
		}
		
		expr4 = stmt();
		node = new_node(ND_WHILE, expr1, expr2, expr3, expr4, NULL);
	}else {
    node = expr();
		expect(TK_RESERVED, ";");
  }

	return node;
}

// expr       = assign
Node_t *expr(void) {
  DEBUG_WRITE("\n");
	return assign();
}

// assign     = equality ("=" assign)?
Node_t* assign(void){
  DEBUG_WRITE("\n");
	Node_t *node = equality();
	if(TK_RESERVED, consume(TK_RESERVED, "=")){
		node = new_node(ND_ASSIGN, node, assign(), NULL, NULL, NULL);
	}
	return node;
}

// equality   = relational ("==" relational | "!=" relational)*
Node_t* equality(void){
  DEBUG_WRITE("\n");
	Node_t *node = relational();

	for(;;){
		if(consume(TK_RESERVED, "==")){
			node = new_node(ND_EQUALTO, node, relational(), NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "!=")){
			node = new_node(ND_NOT_EQUAL_TO, node, relational(), NULL, NULL, NULL);
		} else{
			return node;
		}
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node_t* relational(void){
  DEBUG_WRITE("\n");
	Node_t *node = add();

	for(;;){
		if(consume(TK_RESERVED, "<")){
			node = new_node(ND_LESS_THAN, node, add(), NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "<=")){
			node = new_node(ND_LESS_THAN_OR_EQUALT_TO, node, add(), NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, ">")){
			node = new_node(ND_LESS_THAN, add(), node, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, ">=")){
			node = new_node(ND_LESS_THAN_OR_EQUALT_TO, add(), node, NULL, NULL, NULL);
		} else {
			return node;
		}
	}
}

// add        = mul ("+" mul | "-" mul)*
Node_t* add(void) {
  DEBUG_WRITE("\n");
	Node_t *node = mul();

	for(;;){
		if(consume(TK_RESERVED, "+")){
			node = new_node(ND_ADD, node, mul(), NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "-")){
			node = new_node(ND_SUB, node, mul(), NULL, NULL, NULL);
		} else{
			return node;
		}
	}
}

// mul        = unary ("*" unary | "/" unary)*
Node_t *mul(void){
  DEBUG_WRITE("\n");
	Node_t *node = unary();
	
	for(;;){
		if(consume(TK_RESERVED, "*")){
			node = new_node(ND_MUL, node, unary(), NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "/")){
			node = new_node(ND_DIV, node, unary(), NULL, NULL, NULL);
		} else{
			return node;
		}
	}
}

// unary      = ("+" | "-")? primary
Node_t* unary(){
  DEBUG_WRITE("\n");
	if(consume(TK_RESERVED, "+")){
		return primary();
	}
	if(consume(TK_RESERVED, "-")){
		return new_node(ND_SUB, new_node_num(0), primary(), NULL, NULL, NULL);
	}
	return primary();
}

// primary    = num | ident | "(" expr ")"
Node_t *primary() {
  DEBUG_WRITE("\n");
	Token_t *tok = consume_ident();
	if(tok != NULL){
  	DEBUG_WRITE("this is ident.\n");
		Node_t *node = new_node(ND_LVAR, NULL, NULL, NULL, NULL, NULL);
		LVar_t *lvar = find_lvar(tok);
		if(lvar){
			// 既に変数が存在する
			node->offset = lvar->offset;
		}else{
			// 新しく変数を定義する
			lvar = calloc(1, sizeof(LVar_t));
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			lvar->offset = locals->offset + 8;
			node->offset = lvar->offset;
			locals = lvar;
		}
		return node;
	}
	
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume(TK_RESERVED, "(")) {
  	Node_t *node = expr();
    expect(TK_RESERVED, ")");
    return node;
  }

  // そうでなければ数値のはず
  DEBUG_WRITE("this is number.\n");
  return new_node_num(expect_number());
}
