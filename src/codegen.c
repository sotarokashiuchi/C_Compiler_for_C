#include "common.h"
#include "codegen.h"

/* グローバル変数 */
/// 一意のラベル生成用変数:
static int labelIndex = 0;
// popとpushの回数を数える
static int alignmentCount = 0;

void popPrint(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  alignmentCount -= 8;
  vprintf(fmt, ap);
}

void pushPrint(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  alignmentCount += 8;
  vprintf(fmt, ap);
}

void asmPrint(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
}

/// @brief 左辺値の評価(アドレス計算)
/// @param node 評価対象のノード
void gen_lval(Node_t *node){
  if(node->kind == ND_LVAR){
    asmPrint("  mov rax, rbp\n");
    asmPrint("  sub rax, %d\n", node->identifier->offset);
    pushPrint("  push rax\n");
  } else if(node->kind == ND_DEREF){
    gen(node->expr1);
  } else if(node->kind != ND_LVAR){
    fprintf(stderr, "代入の左辺値が変数ではありません\n");
    return;
  }
}

/// @brief lvalの名前の文字列を取得
/// @param node lvar名を求めたいノード
/// @return 取得した文字列の先頭ポインタ(文字列の末尾には'\0'が格納されている)
char* gen_lval_name(Node_t *node){
  int i;
  char *ident_name = calloc(node->identifier->len+1, sizeof(char));
  for(i=0; i < node->identifier->len; i++){
    ident_name[i] = node->identifier->name[i];
  }
  ident_name[i] = '\0';
  return ident_name;
}

/// @brief アライメントを正しくする
/// @param byte 揃えたいアライメントの境界を指定
int setAlignment(int byte){
  int displacement = alignmentCount%byte;
  // printf("#alignmentCount = %d\n", alignmentCount);
  alignmentCount += displacement;
  asmPrint("  sub rsp, %d\n", displacement);
  return displacement;
}

void gen(Node_t *node) {
  // ローカル変数にアセンブリのラベル識別用の変数を定義することで、
  // 再帰的に読み込んでも値を保持できる
  int lavelIndexLocal = labelIndex++;
  switch (node->kind){
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
            popPrint("  pop rdi\n");
            break;
          case 2:
            popPrint("  pop rsi\n");
            break;
            break;
          case 3:
            popPrint("  pop rdx\n");
            break;
          case 4:
            popPrint("  pop rcx\n");
            break;
          case 5:
            popPrint("  pop r8\n");
            break;
          case 6:
            popPrint("  pop r9\n");
            break;
          default:
            break;
        }
      }
    }
    int displacement = setAlignment(16);
    asmPrint("  call %s\n", gen_lval_name(node));
    asmPrint("  add rsp, %d\n", displacement);
    alignmentCount -= displacement;
    for(int i=7; i<=numOfArgu; i++){
      popPrint("  pop rdi\n");
    }
    asmPrint("  #戻り値をpush\n");
    pushPrint("  push rax\n");
    return;
  }
  case ND_FUNCDEFINE:{
    alignmentCount = 8;
    asmPrint("%s:\n", gen_lval_name(node));
    asmPrint("  #プロローグ\n");
    pushPrint("  push rbp\n");
    asmPrint("  mov rbp, rsp\n");
    asmPrint("  sub rsp, %d\n", local_variable_stack);
    if(node->vector != NULL){
      // 引数がある場合
      asmPrint("#仮引数に実引数を代入\n");
      int i;
      Vector_t *vector = node->vector;
      for(i=1; ; i++){
        gen_lval(vector->node);
        popPrint("  pop rax\n");
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
  case ND_BLOCK:{
    Vector_t *vector = node->vector;
    for( ; ; ){
      gen(vector->node);
      popPrint("  pop rax\n");
      if(vector->next == NULL){
        break;
      }
      vector = vector->next;
    }
    return;
  }
  case ND_ELSE:{
    gen(node->expr1->expr1);
    popPrint("  pop rax\n");
    asmPrint("  cmp rax, 0\n");
    asmPrint("  je .else_%03d\n", lavelIndexLocal);
    gen(node->expr1->expr2);
    asmPrint("  jmp .ifelseend_%03d\n",lavelIndexLocal);

    asmPrint(".else_%03d:\n", lavelIndexLocal);
    gen(node->expr2);
    asmPrint(".ifelseend_%03d:\n", lavelIndexLocal);
    return;
  }
  case ND_IF:{
    gen(node->expr1);
    popPrint("  pop rax\n");
    asmPrint("  cmp rax, 0\n");
    asmPrint("  je  .ifend_%03d\n", lavelIndexLocal);    // ifの条件式が偽の場合jmp
    gen(node->expr2);

    asmPrint(".ifend_%03d:\n", lavelIndexLocal);
    return;
  }
  case ND_WHILE:{
    asmPrint(".while_%03d:\n", lavelIndexLocal);
    gen(node->expr1);
    popPrint("  pop rax\n");
    asmPrint("  cmp rax, 0\n");
    asmPrint("  je  .whileend_%03d\n", lavelIndexLocal);    // whileの条件式が偽の場合jmp
    gen(node->expr2);
    asmPrint("  jmp .while_%03d\n", lavelIndexLocal);

    asmPrint(".whileend_%03d:\n", lavelIndexLocal);
    return;
  }
  case ND_FOR:{
    if(node->expr1 != NULL){
      gen(node->expr1);
    }
    asmPrint(".for_%03d:\n", lavelIndexLocal);
    if(node->expr2 != NULL){
      gen(node->expr2);
    }
    popPrint("  pop rax\n");
    asmPrint("  cmp rax, 0\n");
    asmPrint("  je  .forend_%03d\n", lavelIndexLocal);    // forの条件式が偽の場合jmp
    
    gen(node->expr4);
    if(node->expr3 != NULL){
      gen(node->expr3);
    }
    asmPrint("  jmp .for_%03d\n", lavelIndexLocal);

    asmPrint(".forend_%03d:\n", lavelIndexLocal);
    return;
  }
  case ND_RETURN:{
    // returnは右方方向の木構造しかない
    gen(node->expr2);
    popPrint("  pop rax\n");
    asmPrint("  #エピローグ\n");
    asmPrint("  mov rsp, rbp\n");
    popPrint("  pop rbp\n");
    asmPrint("  ret\n");
    return;
  }
  case ND_NUM:{
    pushPrint("  push %d\n", node->val);
    return;
  }
  case ND_LVAR:{
    gen_lval(node);
    popPrint("  pop rax\n");
    asmPrint("  mov rax, [rax]\n");
    pushPrint("  push rax\n");
    return;
  }
  case ND_ADDR:{
    asmPrint("  #ND_ADDR\n");
    gen_lval(node->expr1);
    return;
  }
  case ND_DEREF:{
    asmPrint("  #ND_DEREF\n");
    gen(node->expr1);
    popPrint("  pop rax\n");
    asmPrint("  mov rax, [rax]\n");
    asmPrint("  push rax\n");
    return;
  }
  case ND_ASSIGN:{
    // 左辺の評価
    gen_lval(node->expr1);
    // 右辺の評価
    gen(node->expr2);

    popPrint("  pop rdi\n");
    popPrint("  pop rax\n");
    // 変数への代入
    asmPrint("  mov [rax], rdi\n");
    pushPrint("  push rdi\n");
    return;
  }
  }

  gen(node->expr1);
  gen(node->expr2);

  popPrint("  pop rdi\n");
  popPrint("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:{
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

  pushPrint("  push rax\n");
}