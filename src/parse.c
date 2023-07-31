#include "common.h"
#include "parse.h"
#include "tokenize.h"

/* EBNF
 * program    = stmt* 
 * stmt    		= expr ";"
 *  					| ident ("(" parmlist ")")?
 * 						| ident ("(" ")")?
 * 						| "return" expr ";"
 * 						| "if" "(" expr ")" stmt ("else" stmt)?
 * 						| "while" "(" expr ")" stmt
 * 						| "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *  					| "{" stmt* "}"
 * parmlist		= expr? | expr ("," expr)?
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
/// @param vector ベクタ
/// @return 生成したノード
Node_t *new_node(NodeKind kind, Node_t *expr1, Node_t *expr2, Node_t *expr3, Node_t *expr4, Node_t *expr5, Vector_t *vector){
	Node_t *node = calloc(1, sizeof(Node_t));
	node->kind = kind;
	node->expr1 = expr1;
	node->expr2 = expr2;
	node->expr3 = expr3;
	node->expr4 = expr4;
	node->expr5 = expr5;
	node->vector = vector;
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

/// @brief 新しいベクタを生成する
/// @param node ベクタに登録するノード
/// @param current 単方リストの現在のベクタ(リストの生成の場合はNULL)
Vector_t* new_vector(Node_t *node, Vector_t *current){
	Vector_t *vector = calloc(1, sizeof(Vector_t));
	if(current!=NULL){
		current->next = vector;
	}
	vector->node = node;
	vector->next = NULL;
	return vector;
}

/// @brief 新しいラベルを生成する
/// @param node 
/// @param tok 識別子を指すトークン
/// @return 生成したラベル
LVar_t* new_lvar(Token_t *tok){
	// 新しく変数を定義する	
	LVar_t *lvar = calloc(1, sizeof(LVar_t));
	lvar->next = identHead;
	lvar->name = tok->str;
	lvar->len = tok->len;
	lvar->offset = identHead->offset + 8;
	identHead = lvar;
	return lvar;
}

/// @brief 識別子の管理を行う(新しい識別子の定義、識別子ごとのメモリの確保)
/// @param kind ノードの種類
/// @param tok 識別子を指すトークン
/// @return 設定完了後のノード
Node_t* manage_lvar(NodeKind kind, Token_t *tok){
	DEBUG_WRITE("this is ident.\n");
	Node_t *node = new_node(kind, NULL, NULL, NULL, NULL, NULL, NULL);
	LVar_t *lvar = find_lvar(tok);
	if(lvar){
		// 既に識別子が存在する
		// node->offset = lvar->offset;
		node->lvar = lvar;
	}else{
		// 新しく識別子を定義する
		lvar = new_lvar(tok);
		// node->offset = lvar->offset;
		node->lvar = lvar;
	}
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
//  				| ident ("(" expr? | expr ("," expr)? ")")?
// 					| "return" expr ";"
// 					| "if" "(" expr ")" stmt ("else" stmt)?
// 					| "while" "(" expr ")" stmt
// 					| "for" "(" expr? ";" expr? ";" expr? ")" stmt
//  				| "{" stmt* "}"
Node_t* stmt(void){
  DEBUG_WRITE("\n");
	Node_t *node;
	Node_t *expr1, *expr2, *expr3, *expr4;
	Token_t *tok;
	
  if((tok = consume_ident()) != NULL){
		Vector_t *vector;
		if(consume(TK_RESERVED, "(")){
			node = manage_lvar(ND_FUNCTION, tok);

			if(!consume(TK_RESERVED, ")")){
				vector = new_vector(expr(), NULL);
				node->vector = vector;
				while(consume(TK_RESERVED, ",")){
					vector = new_vector(expr(), vector);
				}
				expect(TK_RESERVED, ")");
			}
			return node;
		}else{
			back_token(tok);
		}
	}
	
	if (consume(TK_KEYWORD, "return")) {
		// "return" expr ";"
		node = new_node(ND_RETURN, NULL, expr(), NULL, NULL, NULL, NULL);
		expect(TK_RESERVED, ";");
		return node;
  }
	
	if(consume(TK_KEYWORD, "if")) {
		// "if" "(" expr ")" stmt
		// 左ノードに条件式を 右ノードに処理を
		expect(TK_RESERVED, "(");
		expr1 = expr();
		expect(TK_RESERVED, ")");
		expr2 = stmt();
		node = new_node(ND_IF, expr1, expr2, NULL, NULL, NULL, NULL);

		if(consume(TK_KEYWORD, "else")){
			// ("else" stmt)?
			// 左ノードにifを 右ノードに処理を
			node = new_node(ND_ELSE, node, stmt(), NULL, NULL, NULL, NULL);
		}
		return node;
	}
	
	if(consume(TK_KEYWORD, "while")){
		// "while" "(" expr ")" stmt
		expect(TK_RESERVED, "(");
		expr1 = expr();
		expect(TK_RESERVED, ")");
		expr2 = stmt();
		node = new_node(ND_WHILE, expr1, expr2, NULL, NULL, NULL, NULL);
		return node;
	}
	
	if(consume(TK_KEYWORD, "for")){
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
		node = new_node(ND_FOR, expr1, expr2, expr3, expr4, NULL, NULL);
		return node;
	}
	
	if(consume(TK_RESERVED, "{")){
		// "{" stmt* "}"
		Vector_t *vector;
		vector = new_vector(stmt(), NULL);
		node = new_node(ND_BLOCK, NULL, NULL, NULL, NULL, NULL, vector);
		while(!consume(TK_RESERVED, "}")){
			vector = new_vector(stmt(), vector);
		}
		return node;
	}

	node = expr();
	expect(TK_RESERVED, ";");
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
		node = new_node(ND_ASSIGN, node, assign(), NULL, NULL, NULL, NULL);
	}
	return node;
}

// equality   = relational ("==" relational | "!=" relational)*
Node_t* equality(void){
  DEBUG_WRITE("\n");
	Node_t *node = relational();

	for(;;){
		if(consume(TK_RESERVED, "==")){
			node = new_node(ND_EQUALTO, node, relational(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "!=")){
			node = new_node(ND_NOT_EQUAL_TO, node, relational(), NULL, NULL, NULL, NULL);
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
			node = new_node(ND_LESS_THAN, node, add(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "<=")){
			node = new_node(ND_LESS_THAN_OR_EQUALT_TO, node, add(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, ">")){
			node = new_node(ND_LESS_THAN, add(), node, NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, ">=")){
			node = new_node(ND_LESS_THAN_OR_EQUALT_TO, add(), node, NULL, NULL, NULL, NULL);
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
			node = new_node(ND_ADD, node, mul(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "-")){
			node = new_node(ND_SUB, node, mul(), NULL, NULL, NULL, NULL);
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
			node = new_node(ND_MUL, node, unary(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "/")){
			node = new_node(ND_DIV, node, unary(), NULL, NULL, NULL, NULL);
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
		return new_node(ND_SUB, new_node_num(0), primary(), NULL, NULL, NULL, NULL);
	}
	return primary();
}

// primary    = num | ident | "(" expr ")"
Node_t *primary() {
  DEBUG_WRITE("\n");
	Token_t *tok = consume_ident();
	if(tok != NULL){
		return manage_lvar(ND_LVAR, tok);
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
