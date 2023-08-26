int main(){
  int x;
  int *y;
  y = &x;
  *y = 5;
  return x;
}
