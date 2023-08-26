int main(){
  int x[10];
  int sum;
  int i;
  sum = 0;
  for(i=0; i<10; i=i+1){
    x[i] = i;
    sum = sum + x[i];
  }
  return sum;
}
