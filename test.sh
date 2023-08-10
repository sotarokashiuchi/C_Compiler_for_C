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
    exit 1
  fi
  
  echo ""
  
}
assert() {
  simple_test "0" "$1" "$2"
  # sub_assert "-g" "$1" "$2"
}

simple_test() {
  option="$1"
  expected="$2"
  input="$3"

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
# assert "5" "
# int main(){
#   int x;
#   int *y;
#   return x;
# }
# "
assert "5" "
int main(){
  int x;
  int *y;
  int z;
  y# = &x;
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


# assert "5" "
# int inc(int x){
#   x=x+1;
#   return x;
# }

# int main(){
#   int x;
#   x=4;
#   return inc(x);
# }
# "

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


assert "3" "
int main(){
  int x[10];
  *x = 1;
  *(x+1) = 3;
  return *(x+1);
}
"

  # *(x+1) = 2;
  # p = x;
  # return *p + *(p+1);

echo All finished