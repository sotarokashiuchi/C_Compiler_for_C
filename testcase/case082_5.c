// #include <stdio.h>
// #include <stdlib.h>
struct Type {
	int kind;
	char *name;
	int len;
	struct Type *inner;
};

int main (void){
	return (sizeof(struct Type) + sizeof(struct Type*))/8;
}
