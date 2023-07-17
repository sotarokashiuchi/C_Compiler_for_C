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
assert 14 "2*(3+4)"
assert 0 "2+2*(3+4*(1-2))"
assert "13" "2++14+-3"
assert "1" "3<4"
# assert "0" "3>4"
assert "1" "3+4<=1*7"
assert "0" "7==8"
assert "0" "8<7"

echo OK