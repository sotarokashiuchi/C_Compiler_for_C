COPTION=-std=c11 -g -I ./include -static

9cc: main.o tokenize.o parse.o codegen.o
	# gcc -std=c11 -g -static -o 9cc main.o tokenize.o parse.o codegen.o
	gcc codegen.o main.o parse.o tokenize.o -o 9cc

main.o:	./src/main.c
	cc  $(COPTION) -c -o main.o ./src/main.c

tokenize.o: ./src/tokenize.c
	cc  $(COPTION) -c -o tokenize.o ./src/tokenize.c

parse.o: ./src/parse.c
	cc  $(COPTION) -c -o parse.o ./src/parse.c

codegen.o: ./src/codegen.c
	cc  $(COPTION) -c -o codegen.o ./src/codegen.c

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean