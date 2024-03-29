#include "common.h"
#include "parse.h"
#include "tokenize.h"
#include <stdio.h>
#include <stdlib.h>
/* グローバル変数 */
// 文ごとの先頭ノードを格納
Node_t *code[CodeSize];
// identifierのリストの先頭ポインタ
Identifier_t *identHead = NULL;
// identifierのoffset
int identHeadOffset = 0;
// stringのリストの先頭ポインタ
StringVector_t *stringHead = NULL;


/* 関数プロトタイプ宣言 */
/* EBNF
 * program    = funcDefine* | declaration | funcPrototype 				# C99に準拠していない
 * funcDefine = typeSpec* declarator ("(" declaration? | declaration ("," declaration)* ")") "{" stmt* "}"
 * stmt    		= expr? ";"
 * 						| "return" expr? ";"
 * 						| "case" conditional_expr ":" stmt
 * 						| "default" : stmt
 * 						| "if" "(" expr ")" stmt ("else" stmt)?
 * 						| "switch" "(" expr ")" stmt
 * 						| "while" "(" expr ")" stmt
 * 						| "for" "(" (expr? ";" | declaration) expr? ";" expr? ")" stmt
 *  					| "{" stmt* "}"
 * 						| declaration
 * declaration= typeSpec declarator? ("=" initializer)? ";"? # よくわからん
 * declarator = "*"* (ident | declarator) ( 											# 変数定義
 * 																					"[" num "]" 					# 配列定義
 * 																				| ("{" ident "}")*' 			# プロトタイプ宣言
 * 																				)*
 * initializer= assign_expr
 * 						| "{" assign_expr? ("," assign_expr)* ","? "}"
 * 						| string
 * typeSpec		= "int" | "char" | "void"
 * 						| ("struct"  | "union") (ident? "{" (typeSpec* declarator+) "}" | ident)
 * typeName		= typeSpec* "*"* ("[" num "]")* 									# C99に準拠していない
 * expr       			= assign_expr
 * assign_expr 			= conditional_expr (("=" | "*=" | "/=" | "%=" | "+=" | "-=") assign_expr)?
 * conditional_expr = logicalOr_expr '("?" expr ":" conditional_expr)?'
 * logicalOr_expr 	= logicalAnd_expr ("||" logicalAnd_expr )*
 * logicalAnd_expr 	= inclusiveOr_expr ("&&" inclusiveOr_expr)*
 * inclusiveOr_expr	= equality_expr
 * equality_expr 		= relational_expr (("==" | "!=") relational_expr)*
 * relational_expr 	= shift_expr (("<" | ">" | "<=" | ">=" ) shift_expr)*
 * shift_expr 			= additive_expr
 * additive_expr 		= multiplicative_expr (( "+" | "-" ) multiplicative_expr)*
 * multiplicative_expr= cast_expr (( "*" | "/" | "%" ) cast_expr)*
 * cast_expr 				= unary_expr '| (typeName) cast_expr'
 * unary_expr 			= postfix_expr
 | ++ unary_expr
 | -- unary_expr
 | ( & | * | + | - | ! ) cast_expr
 | sizeof unary_expr
 | sizeof "(" typeName ")"
 * postfix_expr 		= primary_expr ("[" expr "]"
 | "(" ParamList ")"
 | "." identifier
 | "->" identifier
 | "++"
 | "--" )*
 * primary_expr 		= identifier | num | string | "(" expr ")"
 * ParamList 	= expr? | expr ("," expr)*
 */

void program(void);
Node_t* funcDefine();
Node_t* funcPrototype();
Node_t* stmt(void);
Node_t* declaration(NodeKind);
Node_t* expr(void);
Node_t* assign_expr(void);
Node_t* conditional_expr(void);
Node_t* logicalOr_expr(void);
Node_t* logicalAnd_expr(void);
Node_t* inclusiveOr_expr(void);
Node_t* equality_expr(void);
Node_t* relational_expr(void);
Node_t* shift_expr(void);
Node_t* additive_expr(void);
Node_t* multiplicative_expr(void);
Node_t* cast_expr();
Node_t* unary_expr();
Node_t* postfix_expr(void);
Node_t* primary_expr(void);
Vector_t* paramList(void);
Node_t* declaration(NodeKind kind);

/// @brief 新しいType_tを作成する
/// @param dataType 
/// @param inner 
/// @return 
Types_t* new_type(DataType dataType, Types_t* inner);
Types_t* typeName(void);
Types_t* typeSpec(NodeKind kind);
Identifier_t* declarator(NodeKind kind, Types_t *type);



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
			if(type->array_size == 0){
				size = 8; // 添字のない文字列リテラルの配列用
			} else {
				size = type->array_size * sizeofType(type->inner);
			}
			break;
		case DT_PTR:
			size = 8;
			break;
		case DT_STRUCT:
			size = type->struct_size;
			break;
		case DT_VOID:
			size = 0; // 仮の値
			break;
		default:
			DEBUG_WRITE("dataType = %d\n", type->dataType);
			assert( false && "型のサイズを計算できません");
	}
	return size;
}

/// @brief 新しいラベルを生成する
/// @param labelVar ラベルに含める番号
/// @param labelIndexVar ラベルの通し番号
Label_t* new_label(int labelVar, int labelIndexVar){
	Label_t *label = calloc(1, sizeof(Label_t));
	label->labelIndex = labelIndexVar;
	label->label = labelVar;
	return label;
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
	switch(kind){
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD:
		case ND_ASSIGN_ADD:
		case ND_ASSIGN_SUB:
		case ND_ASSIGN_MUL:
		case ND_ASSIGN_DIV:
		case ND_ASSIGN_MOD:
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
			break;
		case ND_DEREF:
			node->type = expr1->type->inner;
			node->identifier = expr1->identifier;
			break;
		case ND_ADDR:
			node->type = new_type(DT_PTR, expr1->type);
			break;
		case ND_ASSIGN_EQ:								// =
			// 左辺値の型を代入
			node->type = expr1->type;
			break;
		case ND_EQUALTO:									// ==
		case ND_NOT_EQUAL_TO:							// !=
		case ND_LESS_THAN:								// <
		case ND_LESS_THAN_OR_EQUALT_TO:		// <=
		case ND_LOGICAL_AND:							// &&
		case ND_LOGICAL_OR:								// ||
			node->type = new_type(DT_INT, NULL);
			break;
		case ND_STRING: 									// new_string_vector()で型情報等の継承を行っている
		case ND_STRUCT:										// postfix_expr()で型情報等の継承を行っている
		case ND_STRUCT_MEMBER:						// postfix_expr()で型情報等の継承を行っている
			break;
	}
	// ND_LVAR,        // ローカル変数
	// ND_GVAR,        // グローバル変数
	// ND_GVARDEFINE,  // グローバル変数定義
	// ND_RETURN,      // return
	// ND_IF,          // if
	// ND_ELSE,        // else
	// ND_WHILE,       // while
	// ND_FOR,         // for
	// ND_BLOCK,       // block
	// ND_FUNCCALL,    // funcCall
	// ND_FUNCDEFINE,  // funcDefine
	// ND_NUM, 				// 整数
	// ND_SINGLESTMT,	// 文
	// ND_DOUBLESTMT,	// 複文
	// ND_DECLARATION, // 宣言
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
	node->type = new_type(DT_ARRAY, new_type(DT_CHAR, NULL)); // string型はchar型の配列である
	node->type->array_size = 0;  // sizeof("STRING") は未実装
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
Identifier_t* new_identifier(NodeKind kind, Token_t *tok, Types_t *type){
	// 新しく識別子を定義する	
	Identifier_t *identifier = calloc(1, sizeof(Identifier_t));
	if(!tok){
		identifier->type = type;
		identifier->offset = 0;
		return identifier;
	}
	identifier->next = identHead;
	DEBUG_WRITE("this is identifier.------ %.*s \n", tok->len, tok->str);
	identifier->name = tok->str;
	identifier->len = tok->len;
	identifier->type = type;
	identifier->offset = 0;

	switch(kind){
		case ND_LVAR:
			DEBUG_WRITE("sizeofType = %d\n", sizeofType(type));
			int size = sizeofType(type);
			// char型に対応できていない？
			identHeadOffset += (size>=8 ? size : 8);
			identifier->offset = identHeadOffset;
			identifier->kind = IK_LVAR;
			break;
		case ND_STRUCT_MEMBER:
			identifier->kind = IK_STRUCT_MEMBER;
			return identifier;
		case ND_FUNCDEFINE:
			identifier->kind = IK_FUNC;
			break;
		case ND_FUNCDECLARATION:
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
	return identifier;
}

void program(void){
	DEBUG_WRITE("header node\n");
	int i = 0;
	Token_t *tok;
	void* status;
	while(!at_eof()){
		assert(CodeSize > i && "codeの容量が足りません\n");
		tok = token;

		if(consume(TK_KEYWORD, "struct")){
			typeSpec(-1);
			consume_ident(); // tagName
		} else {
			typeSpec(-1);
		}
		while(consume(TK_RESERVED, "*"));
		consume_ident();

		if(consume(TK_RESERVED, "(")){
			consume(TK_RESERVED, ")");
			if(consume(TK_RESERVED, ";")){
				// プロトタイプ宣言
				back_token(tok);
				code[i++] = funcPrototype();
			} else {
				// 関数定義
				back_token(tok);
				code[i++] = funcDefine();
			}
		} else {
			// グローバル変数定義
			back_token(tok);
			code[i++] = declaration(ND_GVARDEFINE);
		}
	}
	code[i] = NULL;
}

Node_t* funcPrototype(){
	Node_t *node;
	Types_t *type;

	type = typeSpec(-1);
	Identifier_t *identifier = declarator(ND_FUNCDECLARATION, type);
	node = new_node(ND_FUNCDEFINE, NULL, NULL, NULL, NULL, NULL, NULL);
	node->type = type;
	node->identifier = identifier;
	node = new_node(ND_DECLARATION, NULL, NULL, NULL, NULL, NULL, NULL);
	expect(TK_RESERVED, "(");
	expect(TK_RESERVED, ")");
	expect(TK_RESERVED, ";");
	return node;
}

Node_t* funcDefine(){
	DEBUG_WRITE("\n");
	Node_t *node;
	Token_t *tok;
	Vector_t *vector;
	Types_t *type;
	int i = 0;

	// 戻り値の型
	type = typeSpec(-1);
	Identifier_t *identifier = declarator(ND_FUNCDEFINE, type);
	node = new_node(ND_FUNCDEFINE, NULL, NULL, NULL, NULL, NULL, NULL);
	node->type = type;
	node->identifier = identifier;

	if(consume(TK_RESERVED, "(")){
		if(!consume(TK_RESERVED, ")")){
			// 引数がある場合 未実装
			// 一つ目の仮引数
			vector = new_vector(declaration(ND_LVAR), NULL);
			node->vector = vector;

			while(consume(TK_RESERVED, ",")){
				// 二つ目以降の仮引数
				vector = new_vector(declaration(ND_LVAR), vector);
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
		if(vector->node->kind != ND_RETURN){
			vector = new_vector(new_node(ND_RETURN, NULL, NULL, NULL, NULL, NULL, NULL), vector);
		}
		return node;
	}else{
		parseError("error call\n");
	}
}

Node_t* stmt(void){
	DEBUG_WRITE("\n");
	Node_t *node;
	Node_t *expr1, *expr2, *expr3, *expr4;
	Token_t *tok;
	Types_t *type;

	if(peek(TK_KEYWORD, "int") || peek(TK_KEYWORD, "char") || peek(TK_KEYWORD, "void") || peek(TK_KEYWORD, "struct")){
		node = declaration(ND_LVAR);
		return node;
	}


	if (consume(TK_KEYWORD, "return")) {
		// "return" expr? ";"
		if(consume(TK_RESERVED, ";")){
			node = new_node(ND_RETURN, NULL, NULL, NULL, NULL, NULL, NULL);
		} else {
			node = new_node(ND_RETURN, NULL, expr(), NULL, NULL, NULL, NULL);
			expect(TK_RESERVED, ";");
		}
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

	if(consume(TK_KEYWORD, "switch")){
		expect(TK_RESERVED, "(");
		expr1 = expr();
		expect(TK_RESERVED, ")");
		expr2 = stmt();
		return new_node(ND_SWITCH, expr1, expr2, NULL, NULL, NULL, NULL);
	}

	if(consume(TK_KEYWORD, "case")){
		expr1 = conditional_expr();
		expect(TK_RESERVED, ":");
		expr2 = stmt();
		return new_node(ND_CASE, expr1, expr2, NULL, NULL, NULL, NULL);
	}

	if(consume(TK_KEYWORD, "default")){
		expect(TK_RESERVED, ":");
		expr2 = stmt();
		return new_node(ND_DEFAULT, NULL, expr2, NULL, NULL, NULL, NULL);
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
		// "for" "(" (expr? ";" | declaration) expr? ";" expr? ")" stmt
		// for(A; B; C)D
		Node_t *node_declaration = NULL;
		expect(TK_RESERVED, "(");
		if(consume(TK_RESERVED, ";")){
			expr1 = NULL;
		}else{
			// Aの読み込み
			if(peek(TK_KEYWORD, "int") || peek(TK_KEYWORD, "char") || peek(TK_KEYWORD, "void") || peek(TK_KEYWORD, "struct")){
				/* for (int i = 0; i < n; i++) { ... } を
				 * {
				 * 	int i = 0;
				 * 	for (; i < n; i++) { ... }
				 * }  */
				node_declaration = declaration(ND_LVAR);
			} else {
				expr1 = expr();
				expect(TK_RESERVED, ";");
			}
		}

		if(consume(TK_RESERVED, ";")){
			expr2 = new_node_num(1);
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
		if(node_declaration != NULL){
			Vector_t *vector;
			vector = new_vector(node_declaration, NULL);
			node = new_node(ND_BLOCK, NULL, NULL, NULL, NULL, NULL, vector);
			vector = new_vector(new_node(ND_FOR, NULL, expr2, expr3, expr4, NULL, NULL), vector);
		} else {
			node = new_node(ND_FOR, expr1, expr2, expr3, expr4, NULL, NULL);
		}
		return node;
	}

	if(consume(TK_RESERVED, "{")){
		// "{" stmt* "}"
		Vector_t *vector=NULL;
		if(!peek(TK_RESERVED, "}")){
			vector = new_vector(stmt(), NULL);
		}
		node = new_node(ND_BLOCK, NULL, NULL, NULL, NULL, NULL, vector);
		while(!consume(TK_RESERVED, "}")){
			vector = new_vector(stmt(), vector);
		}
		return node;
	}

	if(consume(TK_RESERVED, ";")){
		node = new_node_num(1);
	} else {
		node = expr();
		expect(TK_RESERVED, ";");
	}
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

Identifier_t* declarator(NodeKind kind, Types_t *type){
	Token_t *tok;

	// アスタリスクをパース
	while(consume(TK_RESERVED, "*")){
		type = new_type(DT_PTR, type);
	}

	// 変数名?
	tok = consume_ident();

	// 配列を再帰的にパース
	type  = declaration_array(type);

	return new_identifier(kind, tok, type);
}

Node_t* copy_node(NodeKind kind, Node_t *source){
	Node_t *destination = calloc(1, sizeof(Node_t));
	destination->kind = kind;
	destination->type = source->type;
	destination->identifier = source->identifier;
	return destination;
}

Node_t* initializer(Node_t *node_declarator){
	Node_t *node, *return_node;
	Vector_t* vector;
	Token_t *tok;
	vector = new_vector(node_declarator, NULL); // ex)int x;
	return_node = new_node(ND_DOUBLESTMT, NULL, NULL, NULL, NULL, NULL, vector);
	if(consume(TK_RESERVED, "{")){
		// list
		int index = 0;
		do{
			if(peek(TK_RESERVED, "}")){
				break;
			}
			// 左辺
			node = copy_node(node_declarator->expr1->kind, node_declarator->expr1);
			node = new_node(ND_ADD, node, new_node_num(index++), NULL, NULL, NULL, NULL);
			node = new_node(ND_DEREF, node, NULL, NULL, NULL, NULL, NULL);

			// 右辺
			node = new_node(ND_ASSIGN_EQ, node, assign_expr(), NULL, NULL, NULL, NULL); 
			node = new_node(ND_SINGLESTMT, node, NULL, NULL, NULL, NULL, NULL);
			vector = new_vector(node, vector);
		} while(consume(TK_RESERVED, ","));
		expect(TK_RESERVED, "}");
	} else if(node_declarator->expr1->type->dataType == DT_ARRAY && (tok = consume_string())) {
		// 左辺がchar型の配列の時のみ、stringを特殊に扱う。本来はassign_expr()のstringでパースすべきだが、assign_exprのstringは文字列リテラルの先頭アドレスを返し、配列の初期化のときは特殊であるため
		char c;
		for(int index = 0, offset = 0; index <= tok->len; index++){
			// 左辺
			node = copy_node(node_declarator->expr1->kind, node_declarator->expr1);
			node = new_node(ND_ADD, node, new_node_num(index), NULL, NULL, NULL, NULL);
			node = new_node(ND_DEREF, node, NULL, NULL, NULL, NULL, NULL);

			// 右辺
			if(index == tok->len){
				// 文字列の末尾なら、ナル文字を挿入(仕様とは異なるので注意)
				node = new_node(ND_ASSIGN_EQ, node, new_node_num('\0'), NULL, NULL, NULL, NULL); 
			} else {
				if(tok->str[index+offset] == '\\'){
					//  エスケープ文字を処理
					offset++;
					switch(tok->str[index+offset]){
						case '\'': c=0x27; break;
						case '"':  c=0x22; break;
						case '?':  c=0x3F; break;
						case '\\': c=0x5C; break;
						case 'a':  c=0x07; break;
						case 'b':  c=0x8; break;
						case 'f':  c=0xC; break;
						case 'n':  c=0xA; break;
						case 'r':  c=0xD; break;
						case 't':  c=0xB; break;
						case 'v':  c=0x7; break;
						default: exit(1);
					}
				} else {
					c = tok->str[index+offset];
				}
				node = new_node(ND_ASSIGN_EQ, node, new_node_num(c), NULL, NULL, NULL, NULL); 
			}
			node = new_node(ND_SINGLESTMT, node, NULL, NULL, NULL, NULL, NULL);
			vector = new_vector(node, vector);
		}
	} else {
		// 左辺
		node = copy_node(node_declarator->expr1->kind, node_declarator->expr1);

		// 右辺
		node = new_node(ND_ASSIGN_EQ, node, assign_expr(), NULL, NULL, NULL, NULL); 
		node = new_node(ND_SINGLESTMT, node, NULL, NULL, NULL, NULL, NULL);
		vector = new_vector(node, vector);
		node = new_node(ND_DOUBLESTMT, NULL, NULL, NULL, NULL, NULL, vector);
	}
	return return_node;
}

Node_t* declaration(NodeKind kind){
	Node_t *node;
	Types_t *type = typeSpec(kind);
	if(type == NULL){
		return new_node(ND_DECLARATION, NULL, NULL, NULL, NULL, NULL, NULL);
	}
	Identifier_t *identifier = declarator(kind, type);
	node = new_node(kind, NULL, NULL, NULL, NULL, NULL, NULL);
	node->type = identifier->type;
	node->identifier = identifier;

	if((peek(TK_RESERVED, "=") || peek(TK_RESERVED, ";")) && kind != ND_GVARDEFINE){
		node = new_node(ND_SINGLESTMT, node, NULL, NULL, NULL, NULL, NULL);
		if(consume(TK_RESERVED, "=")){
			node = initializer(node);
		}
	}

	consume(TK_RESERVED, ";");
	return node;
}

Node_t *expr(void) {
	DEBUG_WRITE("\n");
	return assign_expr();
}

Node_t* assign_expr(void){
	DEBUG_WRITE("\n");
	Node_t *node = conditional_expr();
	if(TK_RESERVED, consume(TK_RESERVED, "=")){
		node = new_node(ND_ASSIGN_EQ, node, assign_expr(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "*=")){
		node = new_node(ND_ASSIGN_MUL, node, assign_expr(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "/=")){
		node = new_node(ND_ASSIGN_DIV, node, assign_expr(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "%=")){
		node = new_node(ND_ASSIGN_MOD, node, assign_expr(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "+=")){
		node = new_node(ND_ASSIGN_ADD, node, assign_expr(), NULL, NULL, NULL, NULL);
	}
	if(TK_RESERVED, consume(TK_RESERVED, "-=")){
		node = new_node(ND_ASSIGN_SUB, node, assign_expr(), NULL, NULL, NULL, NULL);
	}
	return node;
}

Node_t* conditional_expr(void){
	return logicalOr_expr();
}

Node_t* logicalOr_expr(void){
	// logicalOr_expr 	= logicalAnd_expr ("||" logicalAnd_expr )*
	Node_t *node = logicalAnd_expr();
	while(consume(TK_RESERVED, "||")){
		node = new_node(ND_LOGICAL_OR, node, logicalAnd_expr(), NULL, NULL, NULL, NULL);
	}
	return node;
}

Node_t* logicalAnd_expr(void){
	// logicalAnd_expr 	= inclusiveOr_expr ("&&" inclusiveOr_expr)*
	Node_t *node = inclusiveOr_expr();
	while(consume(TK_RESERVED, "&&")){
		node = new_node(ND_LOGICAL_AND, node, inclusiveOr_expr(), NULL, NULL, NULL, NULL);
	}
	return node;
}

Node_t* inclusiveOr_expr(void){
	return equality_expr();
}

Node_t* equality_expr(void){
	DEBUG_WRITE("\n");
	Node_t *node = relational_expr();

	for(;;){
		if(consume(TK_RESERVED, "==")){
			node = new_node(ND_EQUALTO, node, relational_expr(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "!=")){
			node = new_node(ND_NOT_EQUAL_TO, node, relational_expr(), NULL, NULL, NULL, NULL);
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

Node_t* relational_expr(void){
	DEBUG_WRITE("\n");
	Node_t *node = shift_expr();

	for(;;){
		if(consume(TK_RESERVED, "<")){
			node = new_node(ND_LESS_THAN, node, shift_expr(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "<=")){
			node = new_node(ND_LESS_THAN_OR_EQUALT_TO, node, shift_expr(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, ">")){
			node = new_node(ND_LESS_THAN, shift_expr(), node, NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, ">=")){
			node = new_node(ND_LESS_THAN_OR_EQUALT_TO, shift_expr(), node, NULL, NULL, NULL, NULL);
		} else {
			return node;
		}
	}
}

Node_t* shift_expr(void){
	return additive_expr();
}


Node_t* additive_expr(void) {
	DEBUG_WRITE("\n");
	Node_t *node = multiplicative_expr(); 

	for(;;){
		if(consume(TK_RESERVED, "+")){
			node = new_node(ND_ADD, node, multiplicative_expr(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "-")){
			node = new_node(ND_SUB, node, multiplicative_expr(), NULL, NULL, NULL, NULL);
		} else{
			return node;
		}
	}
}

Node_t *multiplicative_expr(void){
	DEBUG_WRITE("\n");
	Node_t *node = cast_expr();

	for(;;){
		if(consume(TK_RESERVED, "*")){
			node = new_node(ND_MUL, node, cast_expr(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "/")){
			node = new_node(ND_DIV, node, cast_expr(), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "%")){
			node = new_node(ND_MOD, node, cast_expr(), NULL, NULL, NULL, NULL);
		} else {
			return node;
		}
	}
}

Node_t* cast_expr(void){
	return unary_expr();
}

Node_t* unary_expr(){
	DEBUG_WRITE("\n");
	if(consume(TK_RESERVED, "+")){
		return new_node(ND_ADD, new_node_num(0), cast_expr(), NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "-")){
		return new_node(ND_SUB, new_node_num(0), cast_expr(), NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "!")){
		return new_node(ND_EQUALTO, new_node_num(0), cast_expr(), NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "&")){
		return new_node(ND_ADDR, cast_expr(), NULL, NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "*")){
		return new_node(ND_DEREF, cast_expr(), NULL, NULL, NULL, NULL, NULL);
	}

	if(consume(TK_KEYWORD, "sizeof")){
		Types_t* type;
		Token_t* tok;
		tok = peek(TK_RESERVED, "(");
		if (tok) {
			// sizeof ( typeName )
			token = tok->next;
			type = typeName();
			if(type != NULL){
				// typeNameで処理、それ以外はunary_expr()で処理
				expect(TK_RESERVED, ")");
				return new_node_num(sizeofType(type));
			}
		}

		// sizeof unary_expr
		if (tok != NULL){
			token = tok;
		}
		Node_t *node = unary_expr();
		return new_node_num(sizeofType(node->type));
	}

	if(consume(TK_RESERVED, "++")){
		// ++iは(i += 1)と解釈する
		return new_node(ND_ASSIGN_ADD, unary_expr(), new_node_num(1), NULL, NULL, NULL, NULL);
	}
	if(consume(TK_RESERVED, "--")){
		// --iは(i -= 1)と解釈する
		return new_node(ND_ASSIGN_SUB, unary_expr(), new_node_num(1), NULL, NULL, NULL, NULL);
	}
	return postfix_expr();
}


/// @brief typeからstructタグを検索する (struct A { ... }; という宣言ならAのこと)
/// @param type 基本型がDT_STRUCTのtype
/// @return structタグがあれば、そのメンバのリストの先頭Identifier_t*を返す。未宣言のstructの場合はNULLを返す
Identifier_t* find_struct_tag(Types_t *type){
	Types_t *type_end;
	for(type_end=type; type_end->inner != NULL; type_end=type_end->inner);
	if(type_end->dataType == DT_STRUCT){
		DEBUG_WRITE("point\n");
		Identifier_t *identifier_tag;

		// タグの検索
		for(identifier_tag=identHead; identifier_tag; identifier_tag=identifier_tag->next){
			if(identifier_tag->len == type_end->struct_name_len 
					&& !memcmp(type_end->struct_name, identifier_tag->name, identifier_tag->len)) {
				DEBUG_WRITE("this is STRUCT tag.----%.*s\n", identifier_tag->len, identifier_tag->name);
				break;
			}
		}
		return identifier_tag->member_list;
	}
	return NULL;
}

/* postfix_expr 		= primary_expr ("[" expr "]"
	 | "(" ParamList ")"
	 | "." identifier
	 | "->" identifier
	 | "++"
	 :*/
Node_t* postfix_expr(void){
	Node_t* node;
	Types_t* type = NULL;
	node = primary_expr();

	for(;;){
		if(consume(TK_RESERVED, ".") || peek(TK_RESERVED, "->")){
			if(consume(TK_RESERVED, "->")){
				// A->x  (*A).x
				node = new_node(ND_DEREF, node, NULL, NULL, NULL, NULL, NULL);
			}

			Token_t *tok;
			Identifier_t *identifier;
			size_t size;
			Node_t *node_member, *node_var;

			// struct tag の検索
			identifier = find_struct_tag(node->type);

			// memberの検索
			tok = consume_ident();
			for( ; identifier; identifier = identifier->member_list){
				if(identifier->len == tok->len && !memcmp(tok->str, identifier->name, identifier->len) 
						&& identifier->kind == IK_STRUCT_MEMBER){
					DEBUG_WRITE("this is STRUCT member.---%.*s\n", identifier->len, identifier->name);
					break;
				}
			}
			assert(identifier != NULL && "memberがない\n");

			node_member = new_node(ND_STRUCT_MEMBER, NULL, NULL, NULL, NULL, NULL, NULL);
			node_member->identifier = identifier;
			node = new_node(ND_STRUCT, node, node_member, NULL, NULL, NULL, NULL);
			node->type = identifier->type;
		} else 	if(consume(TK_RESERVED, "++")){
			// i++は((i += 1) - 1)と解釈する
			node = new_node(ND_ASSIGN_ADD, node, new_node_num(1), NULL, NULL, NULL, NULL);
			node = new_node(ND_SUB, node, new_node_num(1), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "--")){
			// i--は((i -= 1) + 1)と解釈する
			node = new_node(ND_ASSIGN_SUB, node, new_node_num(1), NULL, NULL, NULL, NULL);
			node = new_node(ND_ADD, node, new_node_num(1), NULL, NULL, NULL, NULL);
		} else if(consume(TK_RESERVED, "[")){
			// 配列
			node = new_node(ND_ADD, node, expr(), NULL, NULL, NULL, NULL);
			node = new_node(ND_DEREF, node, NULL, NULL, NULL, NULL, NULL);
			expect(TK_RESERVED, "]");
		} else if(consume(TK_RESERVED, "(")){
			// 関数呼び出し
			node->kind = ND_FUNCCALL;
			node->vector = paramList();
			expect(TK_RESERVED, ")");
		} else {
			return node;
		}
	}
	return node;
}

Node_t *primary_expr(void) {
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
				// && プロトタイプ宣言されている関数の場合
				node->kind = ND_FUNCCALL;
				node->identifier = identifier;
			} else {
				parseError("存在しない識別子の種類です\n");
			}
			return node;
		}else{
			if(peek(TK_RESERVED, "(")){
				// 外部のファイルから呼び出した関数の場合
				node->identifier = new_identifier(ND_FUNCCALL, tok, new_type(DT_INT, NULL));
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

	if ((tok = consume_character()) != NULL) {
		DEBUG_WRITE("consume_character\n");
		return new_node_num(tok->val);
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

//* typeSpec		= "int" | "char"
//* 						| ("struct"  | "union") (ident? "{" (typeSpec* declarator+) "}" | ident)
Types_t* typeSpec(NodeKind kind){
	Types_t *type;
	// 基本型の読み込み
	if(consume(TK_KEYWORD, "int")){
		type = new_type(DT_INT, NULL);
	} else if(consume(TK_KEYWORD, "char")){
		type = new_type(DT_CHAR, NULL);
	} else if(consume(TK_KEYWORD, "void")){
		type = new_type(DT_VOID, NULL);
	} else if(consume(TK_KEYWORD, "struct") ) {
		Token_t *tok;
		Node_t *node;
		Identifier_t *identifier, *identifier_tag;
		size_t struct_size = 0, size;
		type = new_type(DT_STRUCT, NULL);
		if((tok = consume_ident())){
			// tag name
			if((identifier = find_identifier(tok))){
				identifier_tag = identifier;
			} else {
				identifier = new_identifier(kind, tok, type);
				identifier_tag = identifier;
			}
			type->struct_name = tok->str;
			type->struct_name_len = tok->len;
		}

		if(consume(TK_RESERVED, "{")){
			while(!consume(TK_RESERVED, "}")){
				// member name
				identifier->member_list = declarator(ND_STRUCT_MEMBER, typeSpec(ND_STRUCT_MEMBER));
				size = sizeofType(identifier->member_list->type);
				identifier->member_list->offset = struct_size;
				struct_size += size > 8 ? size : 8;
				identifier = identifier->member_list;
				expect(TK_RESERVED, ";");
			}
			identifier_tag->type->struct_size = struct_size;
			expect(TK_RESERVED, ";");
			return NULL;
		} else {
			type = identifier_tag->type;
		}
	} else {
		return NULL;
		todoError("まだ実装していない基本型です\n");
	}
	return type;
}

Types_t* typeName(void){
	Types_t *type;
	type = typeSpec(-1);

	if(type==NULL){
		return NULL;
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

