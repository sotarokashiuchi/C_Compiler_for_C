// #include <stdio.h>
// #include <stdlib.h>
struct Type {
	int kind;
	char *name;
	int len;
	struct Type *inner;
};

struct Identifer {
	int kind;
	char *name;
	struct Type *type;
};

int main (void){
	struct Type type;
	type.name = "int";
	type.len = 3;
	type.kind = 1;
	struct Identifer identifer;
	identifer.kind = 1;
	identifer.name = "name";
	identifer.type = &type;
	int x = 5;
	for (struct Type *ptr = identifer.type; ptr->kind != 1 ; ptr){
		x++;
	}
	return x;
}
