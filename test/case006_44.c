int main(){
  int x;
  int *y;
  int z;

  z = sizeof x;
  z = sizeof(x);
  z = z + sizeof(y);
  z = z + sizeof(x+3);
  z = z + sizeof(y+3);
  z = z + sizeof(*y);
  z = z + sizeof(&y);
  z = z + sizeof(1);
  z = z + sizeof(sizeof(1));
  return z;
}
