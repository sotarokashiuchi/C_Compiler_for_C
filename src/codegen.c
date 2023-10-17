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

void popPrint(const char *p){
  // va_list ap;
  // va_start(ap, fmt);
  alignmentCount += 8;
  // vprintf(fmt, ap);
  printf("  pop %s	# %d\n", p, alignmentCount-local_variable_stack);
}

void pushPrint(const char *p){
  // va_list ap;
  // va_start(ap, fmt);
  alignmentCount -= 8;
  // vprintf(fmt, ap);
  printf("  push %s	# %d\n", p, alignmentCount-local_variable_stack);
}

void asmPrint(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
}

int getRegNameFromType(Types_t *type){
  switch (type->dataType){
    case DT_INT:
    case DT_CHAR:
    case DT_PTR:
    	return sizeofType(type);
    case DT_ARRAY:
      return 8;
  }
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
	}
	assert((size==4 || size==8) && "failed serch data size");
}


/// @brief lvalの名前の文字列を取得
/// @param node lvar名を求めたいノード
/// @return 取得した文字列の先頭ポインタ(文字列の末尾には'\0'が格納されている)
char* gen_identifier_name(Node_t *node){
  int i;
  char *ident_name = calloc(node->identifier->len+1, sizeof(char));
  for(i=0; i < node->identifier->len; i++){
    ident_name[i] = node->identifier->name[i];
  }
  ident_name[i] = '\0';
  return ident_name;
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
	}else if(node->kind == ND_LVAR){
		// ローカル変数
    asmPrint("  mov rax, rbp\n");
		asmPrint("  sub rax, %d\n", node->identifier->offset);
    pushPrint("rax");
  } else if(node->kind == ND_GVAR){
		// グローバル変数
		asmPrint("	lea rax, [rip+%s]\n", gen_identifier_name(node));
		// asmPrint("	mov rax, DWORD PTR %s[rip]\n", gen_identifier_name(node));
		pushPrint("rax");
  } else if(node->kind == ND_DEREF){
    gen(node->expr1);
  } else {
    codegenError("代入の左辺値が変数ではありません\n");
    return;
  }
}

/// @brief アライメントを正しくする
/// @param byte 揃えたいアライメントの境界を指定
int setAlignment(int byte){
  assert(alignmentCount>=0 && "over the aligmentCount buf\n");
  int displacement = alignmentCount%byte;
  alignmentCount += displacement;
  asmPrint("  sub rsp, %d\n", displacement);
  return displacement;
}

void gens(void) {
	// 初期化処理
  // 前半部分のコード生成
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

	asmPrint(".data\n");
	for(StringVector_t *string=stringHead; string->next; string = string->next){
		asmPrint(".LC%d:\n", string->labelID);
		asmPrint("	.string \"%.*s\"\n", string->len, string->string);
	}
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

  if(node->type != NULL && node->type->dataType == DT_ARRAY){
    gen_address(node);
    return;
  }
  switch (node->kind){
  case ND_DECLARATION:{
  	asmPrint("	#ND_DECLARATION\n");
    return;
	 }
  case ND_SINGLESTMT:{
    gen(node->expr1);
    popPrint("rax");
  	asmPrint("	#end ND_SINGLESTMT\n");
    return;
  }
  case ND_FUNCCALL:{
    int numOfArgu = 0;
    if(node->vector != NULL){
      // 引数がある場合
      Vector_t *vector = node->vector;
      for(numOfArgu=1; ; numOfArgu++){
        gen(vector->node);
        if(vector->prev == NULL){
          break;
        }
        vector = vector->prev;
      }
      asmPrint("#引数詰め込み開始\n");
      for(int i=1 ; i<=numOfArgu; i++){
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
    }
    int displacement = setAlignment(16);
    asmPrint("  call %s\n", gen_identifier_name(node));
    asmPrint("  add rsp, %d\n", displacement);
    alignmentCount -= displacement;
    for(int i=7; i<=numOfArgu; i++){
      popPrint("rdi");
    }
    asmPrint("  #戻り値をスタックに積む\n");
    pushPrint("rax");
    return;
  }
  case ND_GVARDEFINE:{
    asmPrint("\n.data\n");
		asmPrint("%s:\n", gen_identifier_name(node));
    asmPrint("	.zero %d\n", getRegNameFromType(node->type));
		return;
	}
  case ND_FUNCDEFINE:{
    alignmentCount = local_variable_stack+8;
    asmPrint("\n.text\n");
    asmPrint("%s:\n", gen_identifier_name(node));
    asmPrint("  #プロローグ\n");
    pushPrint("rbp");
    asmPrint("  mov rbp, rsp\n");
    asmPrint("  sub rsp, %d\n", local_variable_stack);
    if(node->vector != NULL && node->vector->node->type->dataType != DT_VOID){
      // 引数がある場合
      asmPrint("#仮引数に実引数を代入\n");
      int i;
      Vector_t *vector = node->vector;
      for(i=1; ; i++){
				gen_address(vector->node);
        popPrint("rax");
        switch (i){
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
            asmPrint("  mov rdi, [rbp+%d]\n", 16+(i-7)*8);
            asmPrint("  mov [rax], rdi\n");
            break;
        }
        if(vector->next == NULL){
          break;
        }
        vector = vector->next;
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
    // returnは右方方向の木構造しかない
		if(node->expr2 != NULL){
			gen(node->expr2);
			popPrint("rax");
		}
    asmPrint("  #エピローグ\n");
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
  case ND_GVAR:{
    gen_address(node);
    popPrint("rax");
		size = getRegNameFromType(node->type);
		if(size == 1){
    	asmPrint("  movzx rax, BYTE PTR [rax]\n");
		} else if (size == 4 || size == 8){
			asmPrint("	mov %s, %s[rip]\n", getRegNameFromSize(size, "rax"), gen_identifier_name(node));
		}

    pushPrint("rax");
    return;
	}
	case ND_STRUCT:
  case ND_LVAR:{
    gen_address(node);
    popPrint("rax");
		size = getRegNameFromType(node->type);
		if(size == 1){
    	asmPrint("  movzx rax, BYTE PTR [rax]\n");
		} else if (size == 4 || size == 8){
    	asmPrint("  mov %s, [rax]\n", getRegNameFromSize(size, "rax")); // 32bitレジスタでも自動的に拡張が行われる
		}

    pushPrint("rax");
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
		size = getRegNameFromType(node->type);
		if(size == 1){
    	asmPrint("  movzx rax, BYTE PTR [rax]\n");
		} else if (size == 4 || size == 8){
    	asmPrint("  mov %s, [rax]\n", getRegNameFromSize(size, "rax"));
		}
    pushPrint("rax");
    return;
  }
  case ND_ASSIGN_EQ:{
    // 左辺の評価
    gen_address(node->expr1);
    // 右辺の評価
    gen(node->expr2);

    popPrint("rdi");
    popPrint("rax");
    // 変数への代入
		size = getRegNameFromType(node->type);
		if(size == 1){
    	asmPrint("  mov [rax], %s\n", getRegNameFromSize(size, "rdi"));
		} else if (size == 4 || size == 8){
    	asmPrint("  mov [rax], %s\n", getRegNameFromSize(size, "rdi"));
		}

    pushPrint("rdi");
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
		size = getRegNameFromType(node->expr1->type);
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
		size = getRegNameFromType(node->type);
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
