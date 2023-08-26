int main(){
  int x;
  int *y;
  int z;
  y = &x;
  *y = 5;
  z = *y;
  return z;
}
