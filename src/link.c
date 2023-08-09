#include <stdio.h>
#include <stdlib.h>

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

int print(void){
  printf("text %d \n", 2);
  return 0;
}

int add(int a, int b, int c, int d, int e, int f, int g, int h){
  return a+b+c+d+e+f+g+h;
}

void getAlloc(int **p, int w, int x, int y, int z){
  *p = malloc(20);
  (*p)[0] = w;
  (*p)[1] = x;
  (*p)[2] = y;
  (*p)[3] = z;
}