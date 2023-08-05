#include "common.h"
#include "codegen.h"

/* グローバル変数 */
/// 一意のラベル生成用変数:
static int labelIndex = 0;
// popとpushの回数を数える
static int alignmentCount = 0;

void genPrint(int byte, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  alignmentCount += byte;
  vprintf(fmt, ap);
}

/// @brief 左辺値の評価(アドレス計算)
/// @param node 評価対象のノード
void gen_lval(Node_t *node){
  if(node->kind != ND_LVAR){
    fprintf(stderr, "代入の左辺値が変数ではありません\n");
    return;
  }

  genPrint(0, "  mov rax, rbp\n");
  genPrint(0, "  sub rax, %d\n", node->lvar->offset);
  genPrint(8, "  push rax\n");
}

/// @brief lvalの名前の文字列を取得
/// @param node lvar名を求めたいノード
/// @return 取得した文字列の先頭ポインタ(文字列の末尾には'\0'が格納されている)
char* gen_lval_name(Node_t *node){
  int i;
  char *ident_name = calloc(node->lvar->len+1, sizeof(char));
  for(i=0; i < node->lvar->len; i++){
    ident_name[i] = node->lvar->name[i];
  }
  ident_name[i] = '\0';
  return ident_name;
}

/// @brief アライメントを正しくする
/// @param byte 揃えたいアライメントの境界を指定
int setAlignment(int byte){
  int displacement = alignmentCount%byte;
  printf("#alignmentCount = %d\n", alignmentCount);
  genPrint(displacement, "  sub rsp, %d\n", displacement);
  return displacement;
}

void gen(Node_t *node) {
  // ローカル変数にアセンブリのラベル識別用の変数を定義することで、
  // 再帰的に読み込んでも値を保持できる
  int lavelIndexLocal = labelIndex++;
  switch (node->kind){
  case ND_FUNCCALL:
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
      genPrint(0, "#引数詰め込み開始\n");
      for(int i=1 ; i<=numOfArgu; i++){
        switch (i){
          case 1:
            genPrint(-8, "  pop rdi\n");
            break;
          case 2:
            genPrint(-8, "  pop rsi\n");
            break;
            break;
          case 3:
            genPrint(-8, "  pop rdx\n");
            break;
          case 4:
            genPrint(-8, "  pop rcx\n");
            break;
          case 5:
            genPrint(-8, "  pop r8\n");
            break;
          case 6:
            genPrint(-8, "  pop r9\n");
            break;
          default:
            break;
        }
      }
    }
    int displacement = setAlignment(16);
    genPrint(0, "  call %s\n", gen_lval_name(node));
    genPrint(-displacement, "  add rsp, %d\n", displacement);
    for(int i=7; i<=numOfArgu; i++){
      genPrint(-8, "  pop rdi\n");
    }
    genPrint(0, "  #戻り値をpush\n");
    genPrint(8, "  push rax\n");
    return;
  case ND_FUNCDEFINE:
    alignmentCount = 8;
    genPrint(0, "%s:\n", gen_lval_name(node));
    genPrint(0, "  #プロローグ\n");
    genPrint(8, "  push rbp\n");
    genPrint(0, "  mov rbp, rsp\n");
    genPrint(0, "  sub rsp, %d\n", local_variable_stack);
    if(node->vector != NULL){
      // 引数がある場合
      genPrint(0, "#仮引数に実引数を代入\n");
      int i;
      Vector_t *vector = node->vector;
      for(i=1; ; i++){
        gen_lval(vector->node);
        genPrint(-8, "  pop rax\n");
        switch (i){
          case 1:
            genPrint(0, "  mov [rax], rdi\n");
            break;
          case 2:
            genPrint(0, "  mov [rax], rsi\n");
            break;
          case 3:
            genPrint(0, "  mov [rax], rdx\n");
            break;
          case 4:
            genPrint(0, "  mov [rax], rcx\n");
            break;
          case 5:
            genPrint(0, "  mov [rax], r8\n");
            break;
          case 6:
            genPrint(0, "  mov [rax], r9\n");
            break;
          default:
            // スタックから実引数を仮引数に移動
            genPrint(0, "  mov rdi, [rbp+%d]\n", 16+(i-7)*8);
            genPrint(0, "  mov [rax], rdi\n");
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
  case ND_BLOCK:
    Vector_t *vector = node->vector;
    for( ; ; ){
      gen(vector->node);
      genPrint(-8, "  pop rax\n");
      if(vector->next == NULL){
        break;
      }
      vector = vector->next;
    }
    return;
  case ND_ELSE:
    gen(node->expr1->expr1);
    genPrint(-8, "  pop rax\n");
    genPrint(0, "  cmp rax, 0\n");
    genPrint(0, "  je .else_%03d\n", lavelIndexLocal);
    gen(node->expr1->expr2);
    genPrint(0, "  jmp .ifelseend_%03d\n",lavelIndexLocal);

    genPrint(0, ".else_%03d:\n", lavelIndexLocal);
    gen(node->expr2);
    genPrint(0, ".ifelseend_%03d:\n", lavelIndexLocal);
    return;
  case ND_IF:
    gen(node->expr1);
    genPrint(-8, "  pop rax\n");
    genPrint(0, "  cmp rax, 0\n");
    genPrint(0, "  je  .ifend_%03d\n", lavelIndexLocal);    // ifの条件式が偽の場合jmp
    gen(node->expr2);

    genPrint(0, ".ifend_%03d:\n", lavelIndexLocal);
    return;
  case ND_WHILE:
    genPrint(0, ".while_%03d:\n", lavelIndexLocal);
    gen(node->expr1);
    genPrint(-8, "  pop rax\n");
    genPrint(0, "  cmp rax, 0\n");
    genPrint(0, "  je  .whileend_%03d\n", lavelIndexLocal);    // whileの条件式が偽の場合jmp
    gen(node->expr2);
    genPrint(0, "  jmp .while_%03d\n", lavelIndexLocal);

    genPrint(0, ".whileend_%03d:\n", lavelIndexLocal);
    return;
  case ND_FOR:
    if(node->expr1 != NULL){
      gen(node->expr1);
    }
    genPrint(0, ".for_%03d:\n", lavelIndexLocal);
    if(node->expr2 != NULL){
      gen(node->expr2);
    }
    genPrint(-8, "  pop rax\n");
    genPrint(0, "  cmp rax, 0\n");
    genPrint(0, "  je  .forend_%03d\n", lavelIndexLocal);    // forの条件式が偽の場合jmp
    
    gen(node->expr4);
    if(node->expr3 != NULL){
      gen(node->expr3);
    }
    genPrint(0, "  jmp .for_%03d\n", lavelIndexLocal);

    genPrint(0, ".forend_%03d:\n", lavelIndexLocal);
    return;
  case ND_RETURN:
    // returnは右方方向の木構造しかない
    gen(node->expr2);
    genPrint(-8, "  pop rax\n");
    genPrint(0, "  #エピローグ\n");
    genPrint(0, "  mov rsp, rbp\n");
    genPrint(-8, "  pop rbp\n");
    genPrint(0, "  ret\n");
    return;
  case ND_NUM:
    genPrint(8, "  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    genPrint(-8, "  pop rax\n");
    genPrint(0, "  mov rax, [rax]\n");
    genPrint(8, "  push rax\n");
    return;
  case ND_ASSIGN:
    // 左辺の評価
    gen_lval(node->expr1);
    // 右辺の評価
    gen(node->expr2);

    genPrint(-8, "  pop rdi\n");
    genPrint(-8, "  pop rax\n");
    // 変数への代入
    genPrint(0, "  mov [rax], rdi\n");
    genPrint(8, "  push rdi\n");
    return;
  }

  gen(node->expr1);
  gen(node->expr2);

  genPrint(-8, "  pop rdi\n");
  genPrint(-8, "  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    genPrint(0, "  add rax, rdi\n");
    break;
  case ND_SUB:
    genPrint(0, "  sub rax, rdi\n");
    break;
  case ND_MUL:
    genPrint(0, "  imul rax, rdi\n");
    break;
  case ND_DIV:
    genPrint(0, "  cqo\n");
    genPrint(0, "  idiv rdi\n");
    break;
  case ND_EQUALTO:
    genPrint(0, "  cmp rax, rdi\n");
    genPrint(0, "  sete al\n");
    genPrint(0, "  movzb rax, al\n");
    break;
  case ND_NOT_EQUAL_TO:
    genPrint(0, "  cmp rax, rdi\n");
    genPrint(0, "  setne al\n");
    genPrint(0, "  movzb rax, al\n");
    break;
  case ND_LESS_THAN:
    genPrint(0, "  cmp rax, rdi\n");
    genPrint(0, "  setl al\n");
    genPrint(0, "  movzb rax, al\n");
    break;
  case ND_LESS_THAN_OR_EQUALT_TO:
    genPrint(0, "  cmp rax, rdi\n");
    genPrint(0, "  setle al\n");
    genPrint(0, "  movzb rax, al\n");
    break;
  }

  genPrint(8, "  push rax\n");
}