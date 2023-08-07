#!/bin/bash
assert() {
  option="$1"
  expected="$2"
  input="$3"

  if [ "$option" = "0" ]; then
    ./9cc "$input" > tmp.s 2> /dev/null
  else
    echo "******************************** [information] ********************************"
    echo "Input:$input"
    echo "********************************** [compile] **********************************"
      ./9cc "$input" > tmp.s
    echo "********************************* [assemble] **********************************"
  fi
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
    echo "12345678901234567890"
    echo "$input => $expected expected, but got $actual"
    # exit 1
  fi
  
  echo ""
  echo ""
}

# assert 理想の実行結果 入力データ
assert "0" "3" "
foo(){
  return 0;
}
main(){
  x = 3;
  return x;
}
foo2(){
  return 2;
}
"


assert "0" "0" "
main(){
  x = 3;
  return func_param(x);
}
"

assert "0" "6" "
foo(){
  return 1;
}
main(){
  x = 3 + foo() + 2;
  return x;
}
foo2(){
  return 2;
}
"

assert "0" "34" "
foo(){
  return 1;
}
main(){
  x = 3 + foo() + 2;
  return add(x, 1, 2, 3, 4, 5, 6, 7);
}
"

assert "0" "3" "
func(x, y){
  z = x - y;
  return z;
}

main(){
  i = 4;
  i = func(i, 1);
  return i;
}
"

assert "0" "34" "
fibonacci(n){
  if(n<=0){
    return 0;
  }
  if(n==1){
    return 1;
  } 
  if(n>1) {
    return fibonacci(n-1)+fibonacci(n-2);
  }
}

main(){
  n = 10;
  for(i=0; i<n; i=i+1){
    x = fibonacci(i);
  }
  return x;
}
"

assert "0" "1" "
add(x, y){
  x = x+y;
  return x;
}

main(){
  x=1;
  y=2;
  z = add(x, y);
  return x;
}
"

assert "0" "5" "
int main(){
  int x;
  x=5;
  y=&x;
  z=&y;
  return **z;
}
"

assert "0" "5" "
int main(){
  int x;
  int y;
  int z;
  x=5;
  y=&x;
  z=&y;
  return **z;
}
"

assert "0" "5" "
int inc(x){
  x=x+1;
  return x;
}

int main(){
  int x;
  return inc(x);
}
"

assert "0" "5" "
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

echo All finished