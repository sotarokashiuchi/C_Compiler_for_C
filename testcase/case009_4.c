int main(){
  int *p;
  int *q;
  int x;
  getAlloc(&p, 1, 2, 4, 8);
  q = p+2;
  x = *q;
  return *q;
}
