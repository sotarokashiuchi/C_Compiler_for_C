#include "common.h"
#include "codegen.h"

static int labelIndex = 0;

/// @brief 左辺値の評価(アドレス計算)
/// @param node 
void gen_lval(Node_t *node){
  if(node->kind != ND_LVAR){
    fprintf(stderr, "代入の左辺値が変数ではありません\n");
    return;
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->lvar->offset);
  printf("  push rax\n");
}

char* gen_lval_name(Node_t *node){
  int i;
  char *ident_name = calloc(node->lvar->len+1, sizeof(char));
  for(i=0; i < node->lvar->len; i++){
    ident_name[i] = node->lvar->name[i];
  }
  ident_name[i] = '\0';
  return ident_name;
}

void gen(Node_t *node) {
  // ローカル変数にアセンブリのラベル識別用の変数を定義することで、
  // 再帰的に読み込んでも値を保持できる
  int lavelIndexLocal = labelIndex++;
  switch (node->kind){
  case ND_FUNCTION:
    if(node->vector != NULL){
      // 引数がある場合
      int i;
      Vector_t *vector = node->vector;
      for(i=1; ; i++){
        gen(vector->node);
        if(vector->prev == NULL){
          break;
        }
        vector = vector->prev;
      }
      printf("#引数詰め込み開始%d\n", i);
      for( ; i>0; i--){
        switch (i){
          case 1:
            printf("  pop rdi\n");
            break;
          case 2:
            printf("  pop rsi\n");
            break;
          case 3:
            printf("  pop rdx\n");
            break;
          case 4:
            printf("  pop rcx\n");
            break;
          case 5:
            printf("  pop r8\n");
            break;
          case 6:
            printf("  pop r9\n");
            break;
          default:
            break;
        }
      }
    }
    printf("  call %s\n", gen_lval_name(node));
    printf("  #戻り値をpush\n");
    printf("  push rax\n");
    return;
  case ND_FUNCDEFINE:
    // printf("  jmp entory\n");
    printf("%s:\n", gen_lval_name(node));
    printf("  #プロローグ\n");
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    // 変数26個分(8byte*26=208byte)の領域を確保する
    printf("  sub rsp, 208\n");
    gen(node->expr1);
    return;
  case ND_BLOCK:
    Vector_t *vector = node->vector;
    for( ; ; ){
      gen(vector->node);
      printf("  pop rax\n");
      if(vector->next == NULL){
        break;
      }
      vector = vector->next;
    }
    return;
  case ND_ELSE:
    gen(node->expr1->expr1);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .else_%03d\n", lavelIndexLocal);
    gen(node->expr1->expr2);
    printf("  jmp .ifelseend_%03d\n",lavelIndexLocal);

    printf(".else_%03d:\n", lavelIndexLocal);
    gen(node->expr2);
    printf(".ifelseend_%03d:\n", lavelIndexLocal);
    return;
  case ND_IF:
    gen(node->expr1);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .ifend_%03d\n", lavelIndexLocal);    // ifの条件式が偽の場合jmp
    gen(node->expr2);

    printf(".ifend_%03d:\n", lavelIndexLocal);
    return;
  case ND_WHILE:
    printf(".while_%03d:\n", lavelIndexLocal);
    gen(node->expr1);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .whileend_%03d\n", lavelIndexLocal);    // whileの条件式が偽の場合jmp
    gen(node->expr2);
    printf("  jmp .while_%03d\n", lavelIndexLocal);

    printf(".whileend_%03d:\n", lavelIndexLocal);
    return;
  case ND_FOR:
    if(node->expr1 != NULL){
      gen(node->expr1);
    }
    printf(".for_%03d:\n", lavelIndexLocal);
    if(node->expr2 != NULL){
      gen(node->expr2);
    }
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .forend_%03d\n", lavelIndexLocal);    // forの条件式が偽の場合jmp
    
    gen(node->expr4);
    if(node->expr3 != NULL){
      gen(node->expr3);
    }
    printf("  jmp .for_%03d\n", lavelIndexLocal);

    printf(".forend_%03d:\n", lavelIndexLocal);
    return;
  case ND_RETURN:
    // returnは右方方向の木構造しかない
    gen(node->expr2);
    printf("  pop rax\n");
    printf("  #エピローグ\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    // 左辺の評価
    gen_lval(node->expr1);
    // 右辺の評価
    gen(node->expr2);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    // 変数への代入
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->expr1);
  gen(node->expr2);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQUALTO:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NOT_EQUAL_TO:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LESS_THAN:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LESS_THAN_OR_EQUALT_TO:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}