#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
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
assert "12" "
a = 3;
b = 5 * 6 - 8;
return a + b / 2;
"

echo OK