#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  echo "******************************** [information] ********************************"
  echo "Input:$input"
  echo "********************************** [compile] **********************************"
  ./9cc "$input" > tmp.s
  echo "********************************* [assemble] **********************************"
  cc -o tmp tmp.s
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
assert "14" "a = 3; b = 5 * 6 - 8; return a + b / 2;"
assert "5" "i=0; if(i==0)i=5; return i;"
assert "6" "i=0; if(i==1)i=5; else i=6; return i;"
# assert "10" "i=0; while(i<10) i=i+1; return i;"

echo finished