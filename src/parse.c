#include "common.h"
#include "parse.h"
#include "tokenize.h"
/* グローバル変数 */
// 文ごとの先頭ノードを格納
Node_t *code[100];
// lvarのリストの先頭ポインタ
Identifier_t *identHead = NULL;


/* 関数プロトタイプ宣言 */
/* EBNF
 * program    = funcDefine*
 * funcDefine = ident ("(" ident? | ident ("," ident)* ")"){ stmt* }
 * stmt    		= expr ";"
 * 						| "return" expr ";"
 * 						| "if" "(" expr ")" stmt ("else" stmt)?
 * 						| "while" "(" expr ")" stmt
 * 						| "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *  					| "{" stmt* "}"
 * 						| typeSpec ident ";"
 * expr       = assign
 * assign     = equality ("=" assign)?
 * equality   = relational ("==" relational | "!=" relational)*
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 * add        = mul ("+" mul | "-" mul)*
 * mul        = unary ("*" unary | "/" unary)*
 * unary      = "+"? primary
							| "-"? primary
							| "*" unary
							| "&" unary
 * primary    = num | ident | funcCall | "(" expr ")"
 * funcCall 	= ident ("(" expr? | expr ("," expr)* ")")
 * typeSpec		= int
 */

// program    = funcDefine*
void program(void);
// funcDefine = ident ("(" ident? | ident ("," ident)* ")"){ stmt* }
Node_t* funcDefine();
// stmt    		= expr ";"
// 						| "return" expr ";"
// 						| "if" "(" expr ")" stmt ("else" stmt)?
// 						| "while" "(" expr ")" stmt
// 						| "for" "(" expr? ";" expr? ";" expr? ")" stmt
//  					| "{" stmt* "}"
Node_t* stmt(void);
// expr       = assign
Node_t* expr(void);
// assign     = equality ("=" assign)?
Node_t* assign(void);
// equality   = relational ("==" relational | "!=" relational)*
Node_t* equality(void);
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node_t* relational(void);
// add        = mul ("+" mul | "-" mul)*
Node_t* add(void);
// mul        = unary ("*" unary | "/" unary)*
Node_t* mul(void);
// unary      = "+"? primary
// 						| "-"? primary
// 						| "*" unary
// 						| "&" unary
Node_t* unary();
// primary    = num | ident | funcCall | "(" expr ")"
Node_t* primary();
// funcCall 	= ident ("(" expr? | expr ("," expr)* ")")
Node_t *funcCall(void);

void typeSpec(void);

Vector_t* parmlist(void);


Identifier_t* find_lvar(Token_t *tok){
	for(Identifier_t *var = identHead; var; var = var->next){
		if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
			return var;
		}
	}
	return NULL;
}

void parseError(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "\x1b[31mParse Error:\x1b[0m");
	vfprintf(stderr, fmt, ap);
	exit(1);
}

void todoError(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "Todo Error:");
	vfprintf(stderr, fmt, ap);
	exit(1);
}

/// @brief ノードを生成する
/// @param kind ノードの種類
/// @param expr1 Node1
/// @param expr2 Node2
/// @param expr3 Node3
/// @param expr4 Node4
/// @param expr5 Node5
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
/// @param current 双方向リストの現在のベクタ(リストの生成の場合はNULL)
Vector_t* new_vector(Node_t *node, Vector_t *current){
	DEBUG_WRITE("\n");
	Vector_t *vector = calloc(1, sizeof(Vector_t));
	if(current!=NULL){
		current->next = vector;
		vector->prev = current;
	}else{
		vector->prev = NULL;
	}
	vector->node = node;
	vector->next = NULL;
	return vector;
}

/// @brief 新しいローカル変数を生成する
/// @param tok 識別子を指すトークン
/// @param type
/// @return 生成したラベル
Node_t* new_identifier(NodeKind kind, Token_t *tok, Types_t *type){
	// 新しく変数を定義する	
	Node_t *node = new_node(kind, NULL, NULL, NULL, NULL, NULL, NULL);
	Identifier_t *identifier = calloc(1, sizeof(Identifier_t));
	identifier->next = identHead;
	identifier->name = tok->str;
	identifier->len = tok->len;
	identifier->type = type;
	if(kind==ND_LVAR){
		identifier->offset = identHead->offset + 8;
	}
	if(kind==ND_FUNCDEFINE || kind==ND_FUNCDEFINE){
		identifier->offset = 0;
	}
	identHead = identifier;
	node->identifier = identifier;
	return node;
}

void program(void){
  DEBUG_WRITE("header node\n");
	int i = 0;
	while(!at_eof()){
		code[i++] = funcDefine();
	}
	code[i] = NULL;
}

Node_t* funcDefine(){
	DEBUG_WRITE("\n");
	Node_t *node;
	Token_t *tok;
	Vector_t *vector;
	int i = 0;
	
	// 戻り値の型
	typeSpec();

  if((tok = consume_ident()) != NULL){
		if(consume(TK_RESERVED, "(")){
			node = new_identifier(ND_FUNCDEFINE, tok, NULL);

			if(!consume(TK_RESERVED, ")")){
				// 引数がある場合 未実装
				// 一つ目の仮引数
				type = typeSpec();
				tok = consume_ident();
				vector = new_vector(new_identifier(ND_LVAR, tok, type), NULL);
				node->vector = vector;

				while(consume(TK_RESERVED, ",")){
					// 二つ目以降の仮引数
					typeSpec();
					tok = consume_ident();
					if(tok != NULL){
						vector = new_vector(new_identifier(ND_LVAR, tok, type), vector);
					}
				}
				expect(TK_RESERVED, ")");
			}
			expect(TK_RESERVED, "{");
			vector = new_vector(stmt(), NULL);
			// Identifier_t dummy = {NULL, NULL, 0, 0};
  		// identHead = &dummy;
			node->expr1 = new_node(ND_BLOCK, NULL, NULL, NULL, NULL, NULL, vector);
			while(!consume(TK_RESERVED, "}")){
				vector = new_vector(stmt(), vector);
			}
			return node;
		}else{
			back_token(tok);
		}
	}
}

Node_t* stmt(void){
  DEBUG_WRITE("\n");
	Node_t *node;
	Node_t *expr1, *expr2, *expr3, *expr4;
	Token_t *tok;
	Types_t *type;

	// 変数宣言
	if(peek(TK_KEYWORD, "int")){
		type = typeSpec();
  	if((tok = consume_ident()) != NULL){
			// node = (ND_LVAR, tok); 
			node = new_identifier(ND_LVAR, tok, type);
			expect(TK_RESERVED, ";");
			return node;
		}else{
			fprintf(stderr, "パーサできません\n");
			exit(1);
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

Node_t *expr(void) {
  DEBUG_WRITE("\n");
	return assign();
}

Node_t* assign(void){
  DEBUG_WRITE("\n");
	Node_t *node = equality();
	if(TK_RESERVED, consume(TK_RESERVED, "=")){
		node = new_node(ND_ASSIGN, node, assign(), NULL, NULL, NULL, NULL);
	}
	return node;
}

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

Vector_t* parmlist(void){
	Vector_t *vector = new_vector(expr(), NULL);
	while(consume(TK_RESERVED, ",")){
		vector = new_vector(expr(), vector);
	}
	return vector;
}

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

Node_t* unary(){
  DEBUG_WRITE("\n");
	if(consume(TK_RESERVED, "+")){
		return primary();
	}
	if(consume(TK_RESERVED, "-")){
		return new_node(ND_SUB, new_node_num(0), primary(), NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "&")){
		return new_node(ND_ADDR, unary(), NULL, NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "*")){
		return new_node(ND_DEREF, unary(), NULL, NULL, NULL, NULL, NULL);
	}
	return primary();
}

Node_t *primary() {
  DEBUG_WRITE("\n");
	Token_t *tok = consume_ident();
	if(tok != NULL){
		if(consume(TK_RESERVED, "(")){
			// function
			back_token(tok);
			return funcCall();
		}else{
			// 変数
			// return manage_lvar(ND_LVAR, tok);
			Node_t *node = new_node(ND_LVAR, NULL, NULL, NULL, NULL, NULL, NULL);
			Identifier_t *identifier = find_lvar(tok);
			if(identifier){
				// 既に識別子が存在する
				node->identifier = identifier;
				return node;
			}else{
				fprintf(stderr, "宣言されていない変数です\n");
				exit(1);
			}
		}
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

void typeSpec(void){
	if(consume(TK_KEYWORD, "int")){
		return;
	}else{
		fprintf(stderr, "データ型がありません。\n");
		exit(1);
	}
}

Node_t *funcCall(void){
	DEBUG_WRITE("\n");
	Node_t *node;
	Token_t *tok;
	
  if((tok = consume_ident()) != NULL){
		Vector_t *vector;
		if(consume(TK_RESERVED, "(")){
			node = new_identifier(ND_FUNCCALL, tok, NULL);

			if(!consume(TK_RESERVED, ")")){
				node->vector = parmlist();
				expect(TK_RESERVED, ")");
			}
			
			return node;
		}else{
			back_token(tok);
			return NULL;
		}
	}
}