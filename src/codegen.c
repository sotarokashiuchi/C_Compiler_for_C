#include "common.h"
#include "codegen.h"
#include "parse.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* グローバル変数 */
/// 一意のラベル生成用変数:
static int labelIndex = 0;
// popとpushの回数を数える
static int alignmentCount = 0;

/* プロトタイプ宣言 */
void gen(Node_t *node);

void codegenError(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "\x1b[31mCodeGen Error:\x1b[0m");
	vfprintf(stderr, fmt, ap);
	exit(1);
}

void asmTab(){
	for(int i=0; i<abs((alignmentCount-local_variable_stack)/8); i++){
		//printf("\t");
	}
}

void popPrint(const char *p){
	// va_list ap;
	// va_start(ap, fmt);
	alignmentCount += 8;
	// vprintf(fmt, ap);
	asmTab();
	printf("  pop %s	# %d\n", p, alignmentCount-local_variable_stack);
}

void pushPrint(const char *p){
	// va_list ap;
	// va_start(ap, fmt);
	alignmentCount -= 8;
	// vprintf(fmt, ap);
	asmTab();
	printf("  push %s	# %d\n", p, alignmentCount-local_variable_stack);
}

void asmPrint(char *fmt, ...){
	asmTab();
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
}


char* getRegNameFromSize(int size, char const *register_name){
	if(!strncmp(register_name, "rax", 3)){
		// rax
		if(size==1){
			return "al";
		}
		if(size==4){
			return "eax";
		}
		if(size==8){
			return "rax";
		}
	} else if(!strncmp(register_name, "rdi", 3)){
		// rdi
		if(size==1){
			return "dil";
		}
		if(size==4){
			return "edi";
		}
		if(size==8){
			return "rdi";
		}
	} else if(!strncmp(register_name, "rsi", 3)){
		// rsi
		if(size==1){
			return "sil";
		}
		if(size==4){
			return "esi";
		}
		if(size==8){
			return "rsi";
		}
	}

	assert((size==4 || size==8) && "failed serch data size");
}

/// @brief スタック上にあるデータを指定されたデータ量だけポップする
/// @param size ポップするサイズ
void stack_pop(int size){
	while(size>0){
		popPrint("rax");
		size -= 8;
	}
}

/// @brief 任意のメモリのデータをスタックに複製する
/// @param size 複製するサイズ
/// @param *dest 複製元のメモリアドレスが格納されたレジスタ名
void pushVarToStack(int size, char *src){
	int offset=0;
	printf("# size = %d\n", size);

	if(size == 1){
		asmPrint("  movzx rdi, BYTE PTR [%s]\n", src);
		pushPrint("rdi");
	} else if(size == 4 || size == 8){
		asmPrint("  mov %s, [%s]\n", getRegNameFromSize(size, "rdi"), src); // 32bitレジスタでも自動的に拡張
		pushPrint("rdi");
	} else {
		// 8byte以上の場合は8の倍数であるという仮定
		alignmentCount -= size;
		asmPrint("  sub rsp, %d  # %d\n", size, alignmentCount-local_variable_stack);
		for(int i=0; i<size/8; i++){
			asmPrint("  mov rdi, [%s+%d]\n", src, offset);
			asmPrint("  mov [rsp+%d], rdi\n", offset);
			offset += 8;
		}
	}
}

/// @brief 任意のメモリへスタックのデータを複製する
/// @param size 複製するサイズ
/// @param *dest 複製先のメモリアドレスが格納されたレジスタ名
void popVarFromStack(int size, char *dest){
	int offset=0;
	printf("# size = %d\n", size);
	if(size == 1){
		popPrint("rdi");
		asmPrint("  mov [%s], %s\n", dest, getRegNameFromSize(size, "rdi"));
		pushPrint("rdi");
	} else if(size == 4 || size == 8){
		popPrint("rdi");
		asmPrint("  mov [%s], %s\n", dest, getRegNameFromSize(size, "rdi"));
		pushPrint("rdi");
	} else {
		// 8byte以上の場合は8の倍数であるという仮定
		for(int i=0; i<size/8; i++){
			asmPrint("  mov rdi, [rsp+%d]\n", offset);
			asmPrint("  mov [%s+%d], rdi\n", dest, offset);
			offset += 8;
		}
		// 値をpushせずにそのままスタックに放置することが、式の値を残すことに相当する
	}
}


/// @brief 左辺値の評価(アドレス計算)
/// @param node 評価対象のノード
void gen_address(Node_t *node){
	if(node->kind == ND_STRUCT){
		/* strcut のメモリ配置
		 * struct A { int x; int y; int z;};
		 * +--------+ A   :rbp-(3*8byte)
		 * + int x  |
		 * +--------+ A.y :(rbp-(3*8byte)) + (1*8byte)
		 * + int y  |
		 * +--------+
		 * + int z  |
		 * +--------+ rbp
		 */
		gen_address(node->expr1);
		popPrint("rax");
		asmPrint(" 	add rax, %d\n", node->expr2->identifier->offset);
		pushPrint("rax");
	} else if(node->kind == ND_LVAR){
		// ローカル変数
		asmPrint("  mov rax, rbp\n");
		asmPrint("  sub rax, %d\n", node->identifier->offset);
		pushPrint("rax");
	} else if(node->kind == ND_GVAR){
		// グローバル変数
		asmPrint("	lea rax, [rip+%.*s]\n", node->identifier->len, node->identifier->name);
		pushPrint("rax");
	} else if(node->kind == ND_DEREF){
		gen(node->expr1);
	} else {
		codegenError("代入の左辺値が変数ではありません\n");
		return;
	}
}

void gens(void) {
	// 初期化処理
	// 前半部分のコード生成
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");

	// date部分のコード生成
	asmPrint(".data\n");
	// 文字列リテラル生成
	for(StringVector_t *string=stringHead; string->next; string = string->next){
		asmPrint(".LC%d:\n", string->labelID);
		asmPrint("	.string \"%.*s\"\n", string->len, string->string);
	}
	// グローバル変数生成
	for(Identifier_t *identifier=identHead; identifier; identifier = identifier->next){
		if(identifier->kind == IK_GVAR){
			DEBUG_WRITE("this is GVAR.\n");
			asmPrint("\n.data\n");
			asmPrint("%.*s:\n", identifier->len, identifier->name);
			asmPrint("	.zero %d\n", sizeofType(identifier->type));
		}
	}
	// 関数の識別子を検索
	fflush(stdout);

	for(int i=0; code[i] != NULL; i++){
		gen(code[i]);
	}

	return;
}

void gen(Node_t *node) {
	// ローカル変数にアセンブリのラベル識別用の変数を定義することで、
	// 再帰的に読み込んでも値を保持できる
	int lavelIndexLocal = labelIndex++;
	int size;

	switch (node->kind) {
	case ND_DECLARATION:{
		asmPrint("	#ND_DECLARATION\n");
		return;
	}
	case ND_GVARDEFINE:{
		asmPrint("	\n#ND_GVARDEFINE\n\n");
		return;
	}
	}
	if(node->type != NULL && node->type->dataType == DT_ARRAY){
		gen_address(node);
		return;
	}
	switch (node->kind){
	case ND_SINGLESTMT:{
    gen(node->expr1);
		size = sizeofType(node->expr1->type);
		stack_pop(size);
  	asmPrint("	#end ND_SINGLESTMT\n");
    return;
	}
	case ND_FUNCCALL:{
    int argumentIndex;
		int offsetSize=0;
		int adjustAligment;
		// 戻り値のアドレス確保
		size = sizeofType(node->type);
		size = size<8 ? 8 : size;
		asmPrint("	# ND_FUNCCALL\n");
		asmPrint("	sub rsp, %d\n", size);
		asmPrint("	mov r12, rsp\n");
		alignmentCount -= size;

		// 引数
		Vector_t *vector = node->vector;
		Vector_t *stackVector = NULL;
		Vector_t *stackVectorHead = NULL;
		Vector_t *registerVector = NULL;
		Vector_t *registerVectorHead = NULL;
		for(argumentIndex=0; vector; vector = vector->prev){
			if(argumentIndex > 6){
				assert(0 && "引数が6個以上に対応していません");
			} else if(vector->node->type != NULL 
					&& vector->node->type->dataType != DT_ARRAY && sizeofType(vector->node->type) > 8){
				// 引数をmemoryを介して渡す
				if(stackVector == NULL){
					stackVector = vector;
					stackVectorHead = stackVector;
				} else {
					stackVector->next = vector;
					stackVector = stackVector->next;
					stackVector->next = NULL;
				}
				offsetSize += sizeofType(stackVector->node->type);
			} else {
				// 引数をregisterを介して渡す
				if(registerVector == NULL){
					registerVector = vector;
					registerVectorHead = registerVector;
				} else {
					registerVector->next = vector;
					registerVector = registerVector->next;
					registerVector->next = NULL;
				}
				argumentIndex++;
			}
		}

		
		// アライメント調節
		adjustAligment = alignmentCount%16;
		asmPrint("  sub rsp, %d\n", adjustAligment);
		alignmentCount += adjustAligment;
		// 引数をmemoryを介して渡す
		for(stackVector = stackVectorHead; stackVector; stackVector=stackVector->next){
			gen(stackVector->node);
		}
		// 引数をregisterを介して渡す
		for(registerVector = registerVectorHead; registerVector; registerVector=registerVector->next){
			gen(registerVector->node);
		}

		asmPrint("#引数詰め込み開始\n");
		for(int i=1 ; i<=argumentIndex; i++){
			switch (i){
				case 1:
					popPrint("rdi");
					break;
				case 2:
					popPrint("rsi");
					break;
					break;
				case 3:
					popPrint("rdx");
					break;
				case 4:
					popPrint("rcx");
					break;
				case 5:
					popPrint("r8");
					break;
				case 6:
					popPrint("r9");
					break;
				default:
					break;
			}
		}
		asmPrint("  call %.*s\n", node->identifier->len, node->identifier->name);
		stack_pop(offsetSize+(argumentIndex<=6 ? 0 : argumentIndex-6));

		// アライメント調節
		asmPrint("  add rsp, %d\n", adjustAligment);
		alignmentCount -= adjustAligment;

		asmPrint("  #戻り値をスタックに積む\n");
		// 戻り値のサイズが8byte以下なら、r12のアドレスに代入し直す
		if(size<=8){
			asmPrint("	mov [r12], rax\n");
		}
		return;
	}
	case ND_FUNCDEFINE:{
		alignmentCount = local_variable_stack+8;
		asmPrint("\n.text\n");
		asmPrint("%.*s:\n", node->identifier->len, node->identifier->name);
		asmPrint("  #プロローグ\n");
		pushPrint("rbp");
		asmPrint("  mov rbp, rsp\n");
		asmPrint("  sub rsp, %d\n", local_variable_stack);
		pushPrint("r12");
		if(node->vector != NULL && node->vector->node->type->dataType != DT_VOID){
			// 引数がある場合
			asmPrint("#仮引数に実引数を代入\n");
			int argumentIndex;
			Vector_t *vector = node->vector;
			int offset=16;
			for(argumentIndex=1; vector; vector=vector->next){
				if(vector->node->type->dataType == DT_STRUCT){
					// 8byte以上の引数をメモリを介して代入する
					int size = sizeofType(vector->node->type);
					// rbpの下位アドレスにある引数(memory argument)のoffsetを計算し、スタック上にコピー
					asmPrint("	mov rax, rbp\n");
					asmPrint("	add rax, %d\n", offset);
					pushVarToStack(size, "rax");
					// 仮引数のメモリを計算し、代入する
					gen_address(vector->node);
					popPrint("rax");
					popVarFromStack(size, "rax");
					stack_pop(size);
					offset += size;
				} else {
					// レジスタを介して引数を受け渡す
					gen_address(vector->node);
					popPrint("rax");
					switch (argumentIndex){
						case 1:
							asmPrint("  mov [rax], rdi\n");
							break;
						case 2:
							asmPrint("  mov [rax], rsi\n");
							break;
						case 3:
							asmPrint("  mov [rax], rdx\n");
							break;
						case 4:
							asmPrint("  mov [rax], rcx\n");
							break;
						case 5:
							asmPrint("  mov [rax], r8\n");
							break;
						case 6:
							asmPrint("  mov [rax], r9\n");
							break;
						default:
							// スタックから実引数を仮引数に移動
							asmPrint("  mov rdi, [rbp+%d]\n", offset);
							asmPrint("  mov [rax], rdi\n");
							offset += 8;
							break;
					}
					argumentIndex++;
				}
			}
		}

		gen(node->expr1);
		return;
	}
	case ND_BLOCK:
	case ND_DOUBLESTMT:{
		Vector_t *vector = node->vector;
		for( ; ; ){
			gen(vector->node);
			if(vector->next == NULL){
				break;
			}
			vector = vector->next;
		}
		return;
	}
	case ND_ELSE:{
		asmPrint("  # if(A) {B} else {C} :A\n");
		gen(node->expr1->expr1);
		popPrint("rax");
		asmPrint("  cmp rax, 0\n");
		asmPrint("  je .else_%03d\n", lavelIndexLocal);
		asmPrint("  # if(A) {B} else {C} :B\n");
		gen(node->expr1->expr2);
		asmPrint("  jmp .ifelseend_%03d\n",lavelIndexLocal);
		asmPrint(".else_%03d:\n", lavelIndexLocal);
		asmPrint("  # if(A) {B} else {C} :C\n");
		gen(node->expr2);
		asmPrint(".ifelseend_%03d:\n", lavelIndexLocal);
		return;
	}
	case ND_IF:{
		asmPrint("  # if(A) {B} :A\n");
		gen(node->expr1);
		popPrint("rax");
		asmPrint("  cmp rax, 0\n");
		asmPrint("  je  .ifend_%03d\n", lavelIndexLocal);    // ifの条件式が偽の場合jmp
		asmPrint("  # if(A) {B} :B\n");
		gen(node->expr2);

		asmPrint(".ifend_%03d:\n", lavelIndexLocal);
		return;
	}
	case ND_WHILE:{
		asmPrint(".while_%03d:\n", lavelIndexLocal);
		asmPrint("  # while(A) {B} :A\n");
		gen(node->expr1);
		popPrint("rax");
		asmPrint("  cmp rax, 0\n");
		asmPrint("  je  .whileend_%03d\n", lavelIndexLocal);    // whileの条件式が偽の場合jmp
		asmPrint("  # while(A) {B} :B\n");
		gen(node->expr2);
		asmPrint("  jmp .while_%03d\n", lavelIndexLocal);

		asmPrint(".whileend_%03d:\n", lavelIndexLocal);
		return;
	}
	case ND_FOR:{
		if(node->expr1 != NULL){
			asmPrint("  # for(A; B; C;) {D} :A\n");
			gen(node->expr1);
			popPrint("rax");
		}
		asmPrint(".for_%03d:\n", lavelIndexLocal);
		if(node->expr2 != NULL){
			asmPrint("  # for(A; B; C;) {D} :B\n");
			gen(node->expr2);
		}
		popPrint("rax");
		asmPrint("  cmp rax, 0\n");
		asmPrint("  je  .forend_%03d\n", lavelIndexLocal);    // forの条件式が偽の場合jmp

		asmPrint(  "  # for(A; B; C;) {D} :D\n");
		gen(node->expr4);
		if(node->expr3 != NULL){
			asmPrint("  # for(A; B; C;) {D} :C\n");
			gen(node->expr3);
			popPrint("rax");
		}
		asmPrint("  jmp .for_%03d\n", lavelIndexLocal);

		asmPrint(".forend_%03d:\n", lavelIndexLocal);
		return;
	}
	case ND_RETURN:{
		/*  8byte以上の戻り値がある場合
		 *	+----------+ <-rsp
		 *	| expr     |
		 *	| (return  |
		 *	|  value)  |
		 *	+----------+ <-rsp+size
		 *	| r12      | -+
		 *	+----------+  |
		 *	| rbp      |  |
		 *	+----------+  |
		 *	| ret addr |  |
		 *	+----------+  |
		 *	| memory   |  |
		 *	| argument |  |
		 *	+----------+  |
		 *	| memory   |  |
		 *	| argument |  |
		 *	|          |  |
		 *	+----------+ <+
		 *	| return   | 
		 *	| value    |
		 *	|          |  
		 *	+----------+  
		 */
		asmPrint("  #ND_RETURN\n");
		if(node->expr2 != NULL){
			// 戻り値の値
			gen(node->expr2);
			size = sizeofType(node->expr2->type);
			if(size > 8){
				// 8byte以上
				asmPrint("	mov r12, [rsp+%d]\n", size);
				popVarFromStack(size, "r12");
			} else {
				// 8byte
				popPrint("rax");
			}
		}
		asmPrint("  #エピローグ\n");
		asmPrint("	pop rdi\n"); // r12をスタックから削除する
		asmPrint("  mov rsp, rbp\n");
		asmPrint("	pop rbp\n");
		asmPrint("  ret\n");
		return;
	}
	case ND_NUM:{
		char buf[8];
		snprintf(buf, 8, "%d", node->val);
		pushPrint(buf);
		return;
	}
	case ND_STRING:{
		asmPrint("	lea rax, [rip+ .LC%d]\n", node->string->labelID);
		pushPrint("rax");
		return;
	}
	case ND_STRUCT:
	case ND_GVAR:
	case ND_LVAR:{
		gen_address(node);
		popPrint("rax");
		size = sizeofType(node->type);
		pushVarToStack(size, "rax");
		return;
	}
	case ND_ADDR:{
		asmPrint("  #ND_ADDR\n");
		gen_address(node->expr1);
		return;
	}
	case ND_DEREF:{
		asmPrint("  #ND_DEREF\n");
		gen(node->expr1);
		popPrint("rax");
		size = sizeofType(node->type);
		pushVarToStack(size, "rax");
		return;
	}
	case ND_ASSIGN_EQ:{
    // 右辺の評価
		gen(node->expr2);
		// 左辺の評価
		gen_address(node->expr1);

		popPrint("rax"); // copy先 dest
    // 変数への代入
		size = sizeofType(node->type);
		popVarFromStack(size, "rax");
		return;
	}
	case ND_ASSIGN_MUL:
	case ND_ASSIGN_DIV:
	case ND_ASSIGN_MOD:
	case ND_ASSIGN_ADD:
	case ND_ASSIGN_SUB:{
		// a += b
    // aのアドレス(aのアドレスは一度しか計算してはならないa = *f()の時に値がかわる)
		gen_address(node->expr1);

		// aの値を評価 // raxにaのアドレスが格納されている想定 // sizeを考慮していない
		size = sizeofType(node->expr1->type);
		if(size == 1){
			asmPrint("  movzx rax, BYTE PTR [rax]\n");
		} else if (size == 4 || size == 8){
			asmPrint("  mov %s, [rax]\n", getRegNameFromSize(size, "rax")); // 32bitレジスタでも自動的に拡張が行われる
		}
		pushPrint("rax");

		// bの値を評価
		gen(node->expr2);

		// a + bを評価
		popPrint("rdi");  // b
		popPrint("rax");  // a

		switch(node->kind){
			case ND_ASSIGN_MUL:
				asmPrint("  imul rax, rdi\n");
				break;
			case ND_ASSIGN_DIV:
				asmPrint("  cqo\n");
				asmPrint("  idiv rdi\n");
				break;
			case ND_ASSIGN_MOD:
				asmPrint("  cqo\n");
				asmPrint("  idiv rdi\n");
				asmPrint("  mov rax, rdx\n");
				break;
			case ND_ASSIGN_ADD:
				if(node->type->dataType == DT_PTR || node->type->dataType == DT_ARRAY){
					if(node->expr1->type->dataType == DT_INT){
						asmPrint("  imul rax, %d\n", sizeofType(node->type->inner));
					} else {
						asmPrint("  imul rdi, %d\n", sizeofType(node->type->inner));
					}
				}
				asmPrint("  add rax, rdi\n");
				break;
			case ND_ASSIGN_SUB:
				asmPrint("  sub rax, rdi\n");
				break;
		}
		pushPrint("rax");

		// aのアドレスにa+bを代入
		popPrint("rdi");
		popPrint("rax");
		size = sizeofType(node->type);
		if(size == 1){
			asmPrint("  mov [rax], %s\n", getRegNameFromSize(size, "rdi"));
		} else if (size == 4 || size == 8){
			asmPrint("  mov [rax], %s\n", getRegNameFromSize(size, "rdi"));
		}

		pushPrint("rdi");
		return;
	}
	case ND_LOGICAL_AND:{
		// A && B
		// Aの評価
		gen(node->expr1);
		popPrint("rax");
		asmPrint("  cmp rax, 0\n");
		asmPrint("  je .logicalAndFalse_%03d\n", lavelIndexLocal);

		// Bの評価
		gen(node->expr2);
		popPrint("rax");
		asmPrint("  cmp rax, 0\n");
		asmPrint("  je .logicalAndFalse_%03d\n", lavelIndexLocal);
		asmPrint("	mov rax, 1\n");
		asmPrint("  jmp .logicalAndEnd_%03d\n", lavelIndexLocal);

		// False
		asmPrint(".logicalAndFalse_%03d:\n", lavelIndexLocal);
		asmPrint("	mov rax, 0\n");
		// End
		asmPrint(".logicalAndEnd_%03d:\n", lavelIndexLocal);
		pushPrint("rax");
		return;
	}
	case ND_LOGICAL_OR:{
		// A || B
		// Aの評価
		gen(node->expr1);
		popPrint("rax");
		asmPrint("  cmp rax, 0\n");
		asmPrint("  jne .logicalOrTrue_%03d\n", lavelIndexLocal);

		// Bの評価
		gen(node->expr2);
		popPrint("rax");
		asmPrint("  cmp rax, 0\n");
		asmPrint("  jne .logicalOrTrue_%03d\n", lavelIndexLocal);
		asmPrint("	mov rax, 0\n");
		asmPrint("  jmp .logicalOrEnd_%03d\n", lavelIndexLocal);

		// True
		asmPrint(".logicalOrTrue_%03d:\n", lavelIndexLocal);
		asmPrint("	mov rax, 1\n");
		// End
		asmPrint(".logicalOrEnd_%03d:\n", lavelIndexLocal);
		pushPrint("rax");
		return;
	}
	}

	gen(node->expr1);
	gen(node->expr2);

	popPrint("rdi");  // 右辺
	popPrint("rax");  // 左辺

	switch (node->kind) {
	case ND_ADD:{
		if(node->type->dataType == DT_PTR || node->type->dataType == DT_ARRAY){
			if(node->expr1->type->dataType == DT_INT){
				asmPrint("  imul rax, %d\n", sizeofType(node->type->inner));
			} else {
				asmPrint("  imul rdi, %d\n", sizeofType(node->type->inner));
			}
		}
		asmPrint("  add rax, rdi\n");
		break;
	}
	case ND_SUB:{
		asmPrint("  sub rax, rdi\n");
		break;
	}
	case ND_MUL:{
		asmPrint("  imul rax, rdi\n");
		break;
	}
	case ND_DIV:{
		asmPrint("  cqo\n");
		asmPrint("  idiv rdi\n");
		break;
	}
	case ND_MOD:{
		asmPrint("  cqo\n");
		asmPrint("  idiv rdi\n");
		asmPrint("  mov rax, rdx\n");
		break;
	}
	case ND_EQUALTO:{
		asmPrint("  cmp rax, rdi\n");
		asmPrint("  sete al\n");
		asmPrint("  movzb rax, al\n");
		break;
	}
	case ND_NOT_EQUAL_TO:{
		asmPrint("  cmp rax, rdi\n");
		asmPrint("  setne al\n");
		asmPrint("  movzb rax, al\n");
		break;
	}
	case ND_LESS_THAN:{
		asmPrint("  cmp rax, rdi\n");
		asmPrint("  setl al\n");
		asmPrint("  movzb rax, al\n");
		break;
	}
	case ND_LESS_THAN_OR_EQUALT_TO:{
		asmPrint("  cmp rax, rdi\n");
		asmPrint("  setle al\n");
		asmPrint("  movzb rax, al\n");
		break;
	}
	}

	pushPrint("rax");
}
