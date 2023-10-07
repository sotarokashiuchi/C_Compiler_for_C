#include "common.h"
#include "parse.h"
#include "tokenize.h"
#include <stdio.h>
/* グローバル変数 */
// 文ごとの先頭ノードを格納
Node_t *code[100];
// lvarのリストの先頭ポインタ
Identifier_t *identHead = NULL;
// stringのリストの先頭ポインタ
StringVector_t *stringHead = NULL;


/* 関数プロトタイプ宣言 */
/* EBNF
 * program    = funcDefine* | declaration
 * funcDefine = ident ("(" ident? | ident ("," ident)* ")"){ stmt* }
 * stmt    		= expr ";"
 * 						| "return" expr ";"
 * 						| "if" "(" expr ")" stmt ("else" stmt)?
 * 						| "while" "(" expr ")" stmt
 * 						| "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *  					| "{" stmt* "}"
 * 						| declaration
 * declaration= declarator ";"
 * 						| declarator "=" initializer ";"
 * initializer= expr
 * 						| "{" expr? | ("," expr)* "}"
 * declarator = typeSpec ident ("[" num "]")?
 * typeSpec		= ("int" | "char") "*"*
 * expr       = assign
 * assign     = equality (("=" | "*=" | "/=" | "%=" | "+=" | "-=") assign)?
 * equality   = relational ("==" relational | "!=" relational)*
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 * add        = mul ("+" mul | "-" mul)*
 * mul        = unary ("*" unary | "/" unary | "%" unary)*
 * unary      = "+"? postfix
							| "-"? postfix
							| "*" unary
							| "&" unary
							| "++" unary
							| "--" unary
							| "sizeof" unary
							| "sizeof" (typeSpec)
 * postfix 		= primary ( "[" expr "]" | "(" ParamList ")" )*
 * 						| postfix "++"
 * 						| postfix "--"
 * primary    = num | string | ident | "(" expr ")"
 * ParamList 	= expr? | expr ("," expr)*
 */

void program(void);
Node_t* funcDefine();
Node_t* stmt(void);
Node_t* declaration(NodeKind);
Node_t* expr(void);
Node_t* assign(void);
Node_t* equality(void);
Node_t* relational(void);
Node_t* add(void);
Node_t* mul(void);
Node_t* unary();
Node_t* postfix(void);
Node_t* primary(void);
Vector_t* paramList(void);

/// @brief 新しいType_tを作成する
/// @param dataType 
/// @param inner 
/// @return 
Types_t* new_type(DataType dataType, Types_t* inner);
Types_t* typeSpec(void);



Identifier_t* find_identifier(Token_t *tok){
	// ローカル変数の識別子を検索
	for(Identifier_t *identifier = identHead; identifier; identifier = identifier->next){
		if(identifier->len == tok->len && !memcmp(tok->str, identifier->name, identifier->len) && identifier->kind == IK_LVAR){
			DEBUG_WRITE("this is LVAR.\n");
			return identifier;
		}
	}
	// グローバル変数の識別子を検索
	for(Identifier_t *identifier = identHead; identifier; identifier = identifier->next){
		if(identifier->len == tok->len && !memcmp(tok->str, identifier->name, identifier->len) && identifier->kind == IK_GVAR){
			DEBUG_WRITE("this is GVAR.\n");
			return identifier;
		}
	}
	// 関数の識別子を検索
	for(Identifier_t *identifier = identHead; identifier; identifier = identifier->next){
		if(identifier->len == tok->len && !memcmp(tok->str, identifier->name, identifier->len) && identifier->kind == IK_FUNC){
			DEBUG_WRITE("this is FUNC.\n");
			return identifier;
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

int sizeofType(Types_t *type){
	int size;
	switch(type->dataType){
		case DT_INT:
			size = 4;
			break;
		case DT_CHAR:
			size = 1;
			break;
		case DT_ARRAY:
			// if(type->inner)
			size = type->array_size * sizeofType(type->inner);
			break;
		case DT_PTR:
			size = 8;
			break;
	}
	return size;
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
	// typeリストの最後のリストを確認する
	if(kind == ND_ADD || kind == ND_SUB || kind == ND_MUL || kind == ND_DIV || kind == ND_MOD || kind == ND_ASSIGN_MUL || kind == ND_ASSIGN_DIV || kind == ND_ASSIGN_ADD || kind == ND_ASSIGN_SUB || kind == ND_ASSIGN_MOD){
		if(expr1->type->dataType == DT_PTR){
			// ポインタ
			node->type = expr1->type;
		}else if(expr2->type->dataType == DT_PTR){
			// ポインタ
			node->type = expr2->type;
		}else if(expr1->type->dataType == DT_ARRAY){
			// 配列
			node->type = new_type(DT_PTR, expr1->type->inner);
		}else if(expr2->type->dataType == DT_ARRAY){
			// 配列
			node->type = new_type(DT_PTR, expr2->type->inner);
		}else{
			// 整数？
			// expr1, 2 どちらでもよい?
			node->type =  expr1->type;
		}
	}
	
	if(kind == ND_DEREF){
		node->type = expr1->type->inner;
	}

	if(kind == ND_ADDR){
		node->type = new_type(DT_PTR, expr1->type);
	}

	if(kind == ND_ASSIGN_EQ){
		// 左辺値の型を代入
		node->type = expr1->type;
	}

  // ND_LVAR,        // ローカル変数
  // ND_EQUALTO,     // ==
  // ND_NOT_EQUAL_TO,// !=
  // ND_GREATER_THAN,// >
  // ND_LESS_THAN,   // <
  // ND_GREATER_THAN_OR_EQUAL_TO,  // >=
  // ND_LESS_THAN_OR_EQUALT_TO,    // <=
  // ND_RETURN,      // return
  // ND_IF,          // if
  // ND_ELSE,        // else
  // ND_WHILE,       // while
  // ND_FOR,         // for
  // ND_BLOCK,       // block
  // ND_FUNCCALL,    // funcCall
  // ND_FUNCDEFINE,  // funcDefine
  // ND_NUM, 				// 整数
	return node;
}

/// @brief numノード(終端記号)を生成する
/// @param val ノードに設定する数値 
/// @return 生成したノード
Node_t *new_node_num(int val){
	Node_t *node = calloc(1, sizeof(Node_t));
	node->kind = ND_NUM;
	node->val = val;
	node->type = new_type(DT_INT, NULL); // INT型を使用する理由は整数拡張を行うため
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

/// @brief 新しいStringベクタを生成する
/// @param node ベクタに登録するノード
Node_t* new_string_vector(Token_t *tok){
	static int labelID;
  DEBUG_WRITE("this is string, labelID %d\n", labelID);
	Node_t *node = new_node(ND_STRING, NULL, NULL, NULL, NULL, NULL, NULL);
	StringVector_t *stringVector = calloc(1, sizeof(StringVector_t));
	stringVector->string = tok->str;
	stringVector->len = tok->len;
	stringVector->labelID = labelID++;
	stringVector->next = stringHead;
	node->string = stringVector;
	stringHead = stringVector;
	return node;
}


/// @brief 新しい識別子を生成する
/// @param tok 識別子を指すトークン
/// @param type
/// @return 生成した識別子
Node_t* new_identifier(NodeKind kind, Token_t *tok, Types_t *type){
  DEBUG_WRITE("this is identifier.\n");
	// 新しく識別子を定義する	
	Node_t *node = new_node(kind, NULL, NULL, NULL, NULL, NULL, NULL);
	Identifier_t *identifier = calloc(1, sizeof(Identifier_t));
	identifier->next = identHead;
	identifier->name = tok->str;
	identifier->len = tok->len;
	identifier->type = type;
	identifier->offset = 0;
	node->type = type;

	switch(kind){
		case ND_LVAR:
			DEBUG_WRITE("sizeofType = %d\n", sizeofType(type));
			int size = sizeofType(type);
			// char型に対応できていない？
			identifier->offset = identHead->offset + (size>=8 ? size : 8);
			identifier->kind = IK_LVAR;
			break;
		case ND_FUNCDEFINE:
			identifier->kind = IK_FUNC;
			break;
		case ND_FUNCCALL:
			// FuncCallの時に新しい識別子を作成するのは間違っているのではないか？
			identifier->kind = IK_FUNC;
			break;
		case ND_GVARDEFINE:
			identifier->kind = IK_GVAR;
			break;
		default:
			parseError("Not define kind\n");
	}

	identHead = identifier;
	node->identifier = identifier;
	return node;
}

void program(void){
  DEBUG_WRITE("header node\n");
	int i = 0;
	Token_t *tok;
	void* status;
	while(!at_eof()){
		tok = token;
		typeSpec();
		consume_ident();
		status = peek(TK_RESERVED, "(");

		back_token(tok);
		if(status){
			code[i++] = funcDefine();
		} else {
			code[i++] = declaration(ND_GVARDEFINE);
		}
	}
	code[i] = NULL;
}

Node_t* funcDefine(){
	DEBUG_WRITE("\n");
	Node_t *node;
	Token_t *tok;
	Vector_t *vector;
	Types_t *type;
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

	if(peek(TK_KEYWORD, "int") || peek(TK_KEYWORD, "char")){
		node = declaration(ND_LVAR);
		return node;
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
	node = new_node(ND_SINGLESTMT, node, NULL, NULL, NULL, NULL, NULL);
	return node;
}

/// @brief 配列をパースしtypeリストを作成する
/// @param types 基本型と、ポインタが格納されたtypeリスト
/// @return typeリスト
Types_t* declaration_array(Types_t* types){
	Types_t* type;
	int size;
	if(consume(TK_RESERVED, "[")){
		size = expect_number();
		expect(TK_RESERVED, "]");
		type = new_type(DT_ARRAY, declaration_array(types));
		type->array_size = size;
		return type;
	} else {
		return types;
	}
}

Node_t* declarator(NodeKind kind){
	Token_t *tok;
	Types_t *type;

	// 変数宣言 (int *a[5] などは非対応)
	type = typeSpec();
	
	tok = consume_ident();
	assert(tok!=NULL && "変数名がない");

	// 配列を再帰的にパース
	type  = declaration_array(type);

	return new_identifier(kind, tok, type);
}

Node_t* initializer(Node_t *node_declarator){
	Node_t *node;
	if(consume(TK_RESERVED, "{")){
		expect(TK_RESERVED, "=");
		// list
	} else {
		Vector_t* vector;
		vector = new_vector(node_declarator, NULL); // ex)int x;
		// 左辺
		node = new_node(ND_LVAR, NULL, NULL, NULL, NULL, NULL, NULL);
		node->identifier = node_declarator->expr1->identifier;
		node->type = node_declarator->expr1->identifier->type;
		if(node->identifier->kind == IK_LVAR){
			node->kind = ND_LVAR;
		} else if (node->identifier->kind == IK_GVAR){
			node->kind = ND_GVAR;
		} else {
			parseError("存在しない識別子の種類です\n");
		}

		// 右辺
		expect(TK_RESERVED, "=");
		node = new_node(ND_ASSIGN_EQ, node, assign(), NULL, NULL, NULL, NULL); 
		node = new_node(ND_SINGLESTMT, node, NULL, NULL, NULL, NULL, NULL);
		vector = new_vector(node, vector);
		node = new_node(ND_DOUBLESTMT, NULL, NULL, NULL, NULL, NULL, vector);
	}
	return node;
}

 /* declaration= declarator ";"
 * 						| declarator "=" initializer ";"
 * initializer= expr
 * 						| "{" expr? | ("," expr)* "}"
 * declarator = typeSpec ident ("[" num "]")?
 */
Node_t* declaration(NodeKind kind){
	Node_t *node;
	node = declarator(kind);

	if(kind != ND_GVARDEFINE){
		node = new_node(ND_SINGLESTMT, node, NULL, NULL, NULL, NULL, NULL);
	}
	
	if(!consume(TK_RESERVED, ";")){
		node = initializer(node);
		expect(TK_RESERVED, ";");
	}
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
		node = new_node(ND_ASSIGN_EQ, node, assign(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "*=")){
		node = new_node(ND_ASSIGN_MUL, node, assign(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "/=")){
		node = new_node(ND_ASSIGN_DIV, node, assign(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "%=")){
		node = new_node(ND_ASSIGN_MOD, node, assign(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "+=")){
		node = new_node(ND_ASSIGN_ADD, node, assign(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "-=")){
		node = new_node(ND_ASSIGN_SUB, node, assign(), NULL, NULL, NULL, NULL);
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

Vector_t* paramList(void){
  DEBUG_WRITE("\n");
	Vector_t *vector;
	
	if(peek(TK_RESERVED, ")")){
		return NULL;
	} else {
		vector = new_vector(expr(), NULL);
	}

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
		} else if(consume(TK_RESERVED, "%")){
			node = new_node(ND_MOD, node, unary(), NULL, NULL, NULL, NULL);
		} else {
			return node;
		}
	}
}

Node_t* unary(){
  DEBUG_WRITE("\n");
	if(consume(TK_RESERVED, "+")){
		return postfix();
	}
	if(consume(TK_RESERVED, "-")){
		return new_node(ND_SUB, new_node_num(0), postfix(), NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "&")){
		return new_node(ND_ADDR, unary(), NULL, NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "*")){
		return new_node(ND_DEREF, unary(), NULL, NULL, NULL, NULL, NULL);
	}

	if(consume(TK_KEYWORD, "sizeof")){
		Types_t* type;
		Token_t* tok;
		tok = peek(TK_RESERVED, "(");
		if (tok) {
			// sizeof ( typeSpec )
			token = tok->next;
			type = typeSpec();
			if(type != NULL){
				expect(TK_RESERVED, ")");
				return new_node_num(sizeofType(type));
			}
		}

		// sizeof unary
		if (tok != NULL){
			token = tok;
		}
		Node_t *node = unary();
		return new_node_num(sizeofType(node->type));
	}

	if(consume(TK_RESERVED, "++")){
		// ++iは(i += 1)と解釈する
		return new_node(ND_ASSIGN_ADD, unary(), new_node_num(1), NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "--")){
		// --iは(i -= 1)と解釈する
		return new_node(ND_ASSIGN_SUB, unary(), new_node_num(1), NULL, NULL, NULL, NULL);
	}
	return postfix();
}

 /* 正しい
 * postfix 		= primary 
 * 						| postfix ( "[" expr "]" | "(" ParamList ")" )*
 * 						| postfix "++"
 * 						| postfix "--"
 /* 現在
 * postfix 		= primary ( "[" expr "]" | "(" ParamList ")" )*
 * 						| postfix "++"
 * 						| postfix "--"
 */
Node_t* postfix(void){
	Node_t* node;
	Types_t* type = NULL;
	node = primary();

	if(consume(TK_RESERVED, "++")){
	// i++は((i += 1) - 1)と解釈する
		node = new_node(ND_ASSIGN_ADD, node, new_node_num(1), NULL, NULL, NULL, NULL);
		return new_node(ND_SUB, node, new_node_num(1), NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "--")){
	// i--は((i -= 1) + 1)と解釈する
		node = new_node(ND_ASSIGN_SUB, node, new_node_num(1), NULL, NULL, NULL, NULL);
		return new_node(ND_ADD, node, new_node_num(1), NULL, NULL, NULL, NULL);
	}

	for( ; ; ){
		if(consume(TK_RESERVED, "[")){
			// 配列
			node = new_node(ND_ADD, node, expr(), NULL, NULL, NULL, NULL);
			node = new_node(ND_DEREF, node, NULL, NULL, NULL, NULL, NULL);
			expect(TK_RESERVED, "]");
			continue;
		} else if(consume(TK_RESERVED, "(")){
			// 関数呼び出し
			node->kind = ND_FUNCCALL;
			node->vector = paramList();
			// プロトタイプ宣言を利用し、戻り値の型を入れるべき(未実装)
			node->type = new_type(DT_INT, NULL);
			expect(TK_RESERVED, ")");
			// 複数回呼べない()
			continue;
		}
		return node;
	}
}

Node_t *primary(void) {
  DEBUG_WRITE("\n");
	Token_t *tok = consume_ident();
	if(tok != NULL){
		// 識別子の読み込み
		Node_t *node = new_node(ND_LVAR, NULL, NULL, NULL, NULL, NULL, NULL);
		Identifier_t *identifier = find_identifier(tok);
		if(identifier){
			// 既に識別子が存在する
			node->identifier = identifier;
			node->type = identifier->type;
			if(identifier->kind == IK_LVAR){
				node->kind = ND_LVAR;
			} else if (identifier->kind == IK_GVAR){
				node->kind = ND_GVAR;
			} else if (identifier->kind == IK_FUNC){
				// 同じファイルで宣言されている関数の場合
				node->kind = ND_FUNCCALL;
			} else {
				parseError("存在しない識別子の種類です\n");
			}
			return node;
		}else{
			if(peek(TK_RESERVED, "(")){
				// 外部のファイルから呼び出した関数の場合
				node->identifier = new_identifier(ND_FUNCCALL, tok, new_type(DT_FUNC, NULL))->identifier;
				node->type = node->identifier->type;
				return node;
			}
			parseError("%*s 宣言されていない変数です\n", tok->len, tok->str);
		}
	}
	
  // "(" expr ")"
  if (consume(TK_RESERVED, "(")) {
  	Node_t *node = expr();
    expect(TK_RESERVED, ")");
    return node;
  }

	if ((tok = consume_string()) != NULL) {
		DEBUG_WRITE("consume_string\n");
		return new_string_vector(tok);
	}

  // 数値
  DEBUG_WRITE("this is number.\n");
  return new_node_num(expect_number());
}

/// @brief 新しいType_tを作成する
/// @param dataType 
/// @param inner 
/// @return 
Types_t* new_type(DataType dataType, Types_t* inner){
  DEBUG_WRITE("\n");
	Types_t* type = calloc(1, sizeof(Types_t));
	type->dataType = dataType;
	type->inner = inner;
	return type;
}

Types_t* typeSpec(void){
	Types_t *type;

	// 基本型の読み込み
	if(consume(TK_KEYWORD, "int")){
		type = new_type(DT_INT, NULL);
	} else if(consume(TK_KEYWORD, "char")){
		type = new_type(DT_CHAR, NULL);
	} else {
		return NULL;
		todoError("まだ実装していない基本型です\n");
	}

	// アスタリスクをパース
	while(consume(TK_RESERVED, "*")){
		type = new_type(DT_PTR, type);
	}

	// 配列を再帰的にパース
	type  = declaration_array(type);
	
	// int*** -> int** -> int* -> int の順に並ぶ
	return type;
}

