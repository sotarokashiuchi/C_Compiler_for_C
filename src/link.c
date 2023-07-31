#include <stdio.h>

int foo(){
  printf("call printf is OK\n");
  return 0;
}

int func_params(int x, int y){
  printf("x + y = %d\n", x+y);
  return 0;
}

int func_param(int x){
  printf("x = %d\n", x);
  return 0;
}