#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
	// 入力データの確認
  if(argc != 2){
    fprintf(stderr, "エラー:引数の個数が正しくありません\n");
    return 1;
  }
	
	// 入力文字列
	char *p = argv[1];

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");
  printf("  mov rax, %ld\n", strtol(p, &p, 10));

	while(*p){
		if(*p == '+'){
			p++;
			printf("\tadd rax, %ld\n", strtol(p, &p, 10));
			continue;
		}
		if(*p == '-'){
			p++;
			printf("\tsub rax, %ld\n", strtol(p, &p, 10));
			continue;
		}
		
		fprintf(stderr, "エラー:予期しない文字です: '%c'\n", *p);
		return 1;
	}
	
  printf("  ret\n");
  return 0;
}