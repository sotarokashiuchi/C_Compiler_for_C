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
    echo "$input => $expected expected, but got $actual"
    # exit 1
  fi
  
  echo ""
  echo ""
}

# assert 理想の実行結果 入力データ
# assert "0" "1" "2*(3+4)+-14==2+2*(3+4*(1-2));"
# assert "0" "9" "abc=3; def=abc*3;"
# assert "0" "3" "
# x = 3;
# y = 5;
# tmp = x;
# x = y;
# y = tmp;
# "
# assert "0" "14" "a = 3; b = 5 * 6 - 8; return a + b / 2;"
# assert "0" "5" "i=0; if(i==0)i=5; return i;"
# assert "0" "6" "i=0; if(i==1)i=5; else i=6; return i;"
# assert "0" "4" "
# i=0;
# if(i==1)i=2;
# else if(i==0)i=4;
#   else i=5;
# return i;
# "
# assert "0" "10" "i=0; while(i<10) i=i+1; return i;"
# assert "0" "15" "
# i=0;
# while(i<10)
#   if(i<5)i=i+1;
#   else i=i+10;
# return i;
# "

# assert "0" "20" "
# a=0;
# for(i=0; i<10; i=i+1)a=a+2;
# return a;
# "

# assert "0" "10" "
# a=0;
# i=0;
# for( ; i<10; )i=i+1;
# return i;
# "

# assert "0" "30" "
# a=0;
# for(i=0; i<10; i=i+1){
#   a=a+1;
#   a=a+2;
# }
# return a;
# "

# assert "0" "18" "
# sum=0;
# based=3;
# for(i=0; i<12; i=i+1){
#   if(i/based==1){
#     sum = sum + i;
#     based = based + 3;
#   }
# }
# return sum;
# "

# assert "0" "2" "
# a = 2;
# foo();
# return a;
# "

# assert "0" "0" "
# x = 3;
# y = 5;
# func_params(x, y);
# "


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
  func_param(x);
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

assert "-g" "36" "
total(a, b, c, d, e, f, g, h){
  x = a+b+c+d+e+f+g+h;
  return x;
}

main(){
  i = total(1, 2, 3, 4, 5, 6, 7, 8);
  return i;
}
"
assert "-g" "34" "
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

echo All finished