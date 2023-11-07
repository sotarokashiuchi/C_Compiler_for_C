COPTION=--pedantic -std=c11 -g -I ./include -static
INCLUDEOBJ=./include/common.h ./include/tokenize.h ./include/parse.h ./include/codegen.h
CODE='int main(void){ int x; x=5; return x; }'
CODE='int main(void){ int x; x=0; x++; return x; }'
CODE='int main(void){ return "Hello"; }'
CODE='int main(void){ return "Hello"; }'
CODE='int foo(void){ return 5;} int main(void){return "Hello"; }'
CODE='int foo(void){ return 5;} int main(void){return foo(); }'

9cc: main.o tokenize.o parse.o codegen.o
	# gcc -std=c11 -g -static -o 9cc main.o tokenize.o parse.o codegen.o
	gcc codegen.o main.o parse.o tokenize.o -o 9cc

main.o:	./src/main.c $(INCLUDEOBJ)
	cc  $(COPTION) -c -o main.o ./src/main.c

tokenize.o: ./src/tokenize.c $(INCLUDEOBJ)
	cc  $(COPTION) -c -o tokenize.o ./src/tokenize.c

parse.o: ./src/parse.c $(INCLUDEOBJ)
	cc  $(COPTION) -c -o parse.o ./src/parse.c

codegen.o: ./src/codegen.c $(INCLUDEOBJ)
	cc  $(COPTION) -c -o codegen.o ./src/codegen.c

test: test.c
	gcc test.c -o test

debug: 9cc test
	DEBUG=1 ./test $(FN)

2kmcc: 9cc test
	DEBUG=1 ./test "2kmcc_1.c"
	./tmp $(CODE) > tmp_2kmcc.s
	cc -static tmp_2kmcc.s -o code_2kmcc
	./code_2kmcc || echo $$?

clean:
	rm -f 9cc test *.o *~ tmp* code_2kmcc

.PHONY: alltest clean debug
