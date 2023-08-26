int main(){
  int x[10];
  *x = 1;
  *(x+1) = 2;
  int *p;
  p = x;
  return *(x+1) + *p + *(p+1);
}
