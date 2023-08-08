#!/bin/bash
sub_assert() {
  option="$1"
  expected="$2"
  input="$3"

  if [ "$option" = "0" ]; then
    ./9cc "$input" > tmp.s
  else
    echo "******************************** [information] ********************************"
    echo "Input:$input"
    echo "********************************** [compile] **********************************"
      CC_DEBUG=1 ./9cc "$input" > tmp.s
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
  
}
assert() {
  sub_assert "0" "$1" "$2"
}

debug_assert() {
  sub_assert "$1" "$2"
}

# assert 理想の実行結果 入力データ
debug_assert "1" "
int add(int x, int y){
  int x;
  int y;
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
  return x;
}
"

debug_assert "5" "
int main(){
  int x;
  x=5;
  y=&x;
  z=&y;
  return **z;
}
"

assert "5" "
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

assert "5" "
int inc(x){
  x=x+1;
  return x;
}

int main(){
  int x;
  return inc(x);
}
"

debug_assert "5" "
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