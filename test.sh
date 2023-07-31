#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  echo "******************************** [information] ********************************"
  echo "Input:$input"
  echo "********************************** [compile] **********************************"
  ./9cc "$input" > tmp.s
  echo "********************************* [assemble] **********************************"
  cc -o link.o -c ./src/link.c
  cc -o tmp.o -c tmp.s
  cc -o tmp link.o tmp.o
  echo "********************************* [execution] *********************************"
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "successful!!"
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    # exit 1
  fi
  
  echo ""
  echo ""
}

# assert 理想の実行結果 入力データ
# assert "1" "2*(3+4)+-14==2+2*(3+4*(1-2));"
# assert "9" "abc=3; def=abc*3;"
# assert "3" "
# x = 3;
# y = 5;
# tmp = x;
# x = y;
# y = tmp;
# "
# assert "14" "a = 3; b = 5 * 6 - 8; return a + b / 2;"
# assert "5" "i=0; if(i==0)i=5; return i;"
# assert "6" "i=0; if(i==1)i=5; else i=6; return i;"
# assert "4" "
# i=0;
# if(i==1)i=2;
# else if(i==0)i=4;
#   else i=5;
# return i;
# "
# assert "10" "i=0; while(i<10) i=i+1; return i;"
# assert "15" "
# i=0;
# while(i<10)
#   if(i<5)i=i+1;
#   else i=i+10;
# return i;
# "

# assert "20" "
# a=0;
# for(i=0; i<10; i=i+1)a=a+2;
# return a;
# "

# assert "10" "
# a=0;
# i=0;
# for( ; i<10; )i=i+1;
# return i;
# "

assert "30" "
a=0;
for(i=0; i<10; i=i+1){
  a=a+1;
  a=a+2;
}
return a;
"

assert "18" "
sum=0;
based=3;
for(i=0; i<12; i=i+1){
  if(i/based==1){
    sum = sum + i;
    based = based + 3;
  }
}
return sum;
"

assert "2" "
a = 2;
foo()
return a;
"

assert "0" "
x = 3;
y = 5;
func_param(x, y)
"

assert "0" "
x = 3;
y = 5;
func_param(x)
"

echo finished