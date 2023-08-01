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

int print(int x){
  printf("%d ", x);
  return 0;
}

int add(int a, int b, int c, int d, int e, int f, int g, int h){
  return a+b+c+d+e+f+g+h;
}