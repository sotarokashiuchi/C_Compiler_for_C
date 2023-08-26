#!/bin/bash
sub_assert() {
  expected="$1"
  input="$2"

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

file=$(cat testcase/case001_5.c)
assert "5" "$file"
# cat test/case001_5.c | sed 's/\n\s//g' | assert "4" 

<< COMMENTOUT


echo All finished
