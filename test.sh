#!/bin/bash
sub_assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  echo "******************************** [information] ********************************"
  echo "Input:$input"
  echo "********************************** [compile] **********************************"
    CC_DEBUG=1 ./9cc "$input" > tmp.s
  echo "********************************* [assemble] **********************************"
  cc -o link.o -c ./src/link.c
  cc -o tmp.o -c tmp.s
  cc -o tmp link.o tmp.o
  echo "********************************* [execution] *********************************"
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
    echo "successful!!"
  else
    # echo "12345678901234567890"
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
  
  echo ""
  
}
assert() {
  if [ "$DEBUG" == "1" ]; then
    sub_assert "$1" "$2"
  else
    simple_test "$1" "$2"
  fi
}

simple_test() {
  expected="$1"
  input="$2"

  CC_DEBUG=0 ./9cc "$input" > tmp.s 2> /dev/null
  cc -o link.o -c ./src/link.c 2> /dev/null
  cc -o tmp.o -c tmp.s 2> /dev/null
  cc -o tmp link.o tmp.o 2> /dev/null
  ./tmp 2> /dev/null
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo -e "\e[42mOK \e[0m $(echo $input | sed 's/\n\s//g') => \e[32m$actual\e[0m" 
  else
    echo -e "\e[41mERR\e[0m $(echo $input | sed 's/\n\s//g') => \e[31m$actual\e[0m" 
  fi
}

# assert 理想の実行結果 入力データ
assert "5" "
int main(){
  int x;
  int *y;
  int z;
  y = &x;
  *y = 5;
  z = *y;
  return z;
}
"

assert "5" "
int main(){
  int x;
  int *y;
  y = &x;
  *y = 5;
  return x;
}
"

assert "5" "
int main(){
  int x;
  int *y;
  int **z;
  y = &x;
  *y = 5;
  z = &y;
  return **z;
}
"

assert "3" "
int main(){
  int x;
  int y;
  x=0;
  y=x+3;
  return y;
}
"

assert "44" "
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
"


assert "3" "
int add(int x, int y){
  x = x+y;
  return x;
}

int main(){
  int x;
  int y;
  int z;
  x=1;
  y=2;
  z = add(x, y);
  return z;
}
"


assert "5" "
int inc(int x){
  x=x+1;
  return x;
}

int main(){
  int x;
  x=4;
  return inc(x);
}
"

assert "4" "
int main(){
  int *p;
  int *q;
  int x;
  getAlloc(&p, 1, 2, 4, 8);
  q = p+2;
  x = *q;
  return *q;
}
"


assert "5" "
int main(){
  int x[10];
  *x = 1;
  *(x+1) = 2;
  int *p;
  p = x;
  return *(x+1) + *p + *(p+1);
}
"

assert "1" "
int main(){
  int x[10];
  x[0] = 1;
  return x[0];
}
"


assert "3" "
int main(){
  int x[10][3];
  x[0][2] = 3;
  return x[0][2];
}
"

assert "5" "
int main(){
  int x;
  x = 12;
  return x%7;
}
"

assert "1" "
int main(){
  int x;
  x = 8;
  return x%7;
}
"

assert "1" "
int main(){
  int x;
  x = 8;
  return x%7;
}
"

assert "55" "
int main(){
  int i;
  int x;
  x=0;
  for(i=0; i<11; i=i+1){
    x = x+i;
  }
  return x;
}
"

assert "4" "
int main(){
  int x[3];
  int i;
  i = 0;
  x[i] = 4;
  return x[i];
}
"

assert "0" "
int main(){
  int i;
  int j;
  int x[10];
  x[0] = 4;
  x[1] = 6;
  x[2] = 5;
  x[3] = 9;
  x[4] = 20;
  x[5] = 30;
  x[6] = 5;
  x[7] = 6;
  x[8] = 8;
  int tmp;
  for(i=0; i<8; i=i+1){
    for(j=i+1; j<9; j=j+1){
      if(x[i] > x[j]){
        tmp = x[j];
        x[j] = x[i];
        x[i] = tmp;
      }
    }
  }
  x[9] = -1;
  sortPrint(x);
  return 0;
}
"

# assert "0" " 
# int main() {
#   intPrint(1);
#   int base;     
#   base = 10000;
#   int n;         
#   n = 100;
#   int i;                
#   int temp;             
#   int out;              
#   int denom;            
#   int numerator[8401];  
#   intPrint(2);

#   for (i = 0; i < n; i=i+1) {
#     numerator[i] = base / 5;
#     intPrint(i);
#   }
#   out = 0;
#   for (n = 100; n > 0; n = n-14) {
#     temp = 0;
#     for (i = n - 1; i > 0; i=i-1) {
#       intPrint(i);
#       denom = 2 * i - 1;
#       temp = temp * i + numerator[i] * base;
#       numerator[i] = temp % denom;
#       temp = temp / denom;
#     }
#     intPrint(out + temp / base);
#     out = temp % base;
#   }
#   return 0;
# }
# "
 

echo All finished