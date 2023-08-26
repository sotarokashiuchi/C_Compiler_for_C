COPTION=--pedantic -std=c11 -g -I ./include -static
INCLUDEOBJ=./include/common.h ./include/tokenize.h ./include/parse.h ./include/codegen.h

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

alltest: 9cc test
	./test

debug: 9cc test
	DEBUG=1 ./test $(FN)

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: alltest clean debug
