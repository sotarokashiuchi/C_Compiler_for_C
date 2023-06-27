# CFLAGS=-std=c11 -g -static -o 9cc ./src/main.c

9cc: main.o tokenize.o
	cc -std=c11 -g -I ./include -static -o 9cc main.o tokenize.o

main.o:	./src/main.c
	cc -std=c11 -g -c -I ./include -static -o main.o ./src/main.c

tokenize.o: ./src/tokenize.c
	cc -std=c11 -g -c -I ./include -static -o tokenize.o ./src/tokenize.c

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean