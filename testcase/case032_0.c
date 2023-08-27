int main() {
  int base;     
  base = 10000;
  int n;         
  n = 8400;
  int i;                
  int temp;             
  int out;              
  int denom;            
  int numerator[8401];  

  for (i = 0; i < n; i=i+1) {
    numerator[i] = base / 5;
  }
  out = 0;
  for (n = 8400; n > 0; n = n-14) {
    temp = 0;
    for (i = n - 1; i > 0; i=i-1) {
      denom = 2 * i - 1;
      temp = temp * i + numerator[i] * base;
      numerator[i] = temp % denom;
      temp = temp / denom;
    }
    intPrint(out + temp / base);
    out = temp % base;
  }
  return 0;
}
