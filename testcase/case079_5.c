// #include <stdio.h>
// #include <stdlib.h>
struct Type {
	int kind;
	char *name;
	int len;
	struct Type *inner;
};

struct Type new_type(int kind, char *name, int len, struct Type* typeHead){
	struct Type *newType = malloc(32);
	newType->kind = kind;
	newType->name = name;
	newType->len = len;
	typeHead->inner = newType;
	printf("typeHead\t=%p ->kind=%d, ->name=%s, ->len=%d, ->inner=%p\n", typeHead, typeHead->kind, typeHead->name, typeHead->len, typeHead->inner);
	return *typeHead;
}

int main (void){
	struct Type typeHead;
	typeHead.kind = 1;
	typeHead.len = 2;
	typeHead.name = "name";
	typeHead.inner = NULL;
	typeHead = new_type(3, "names", 4, &typeHead);
	printf("typeHead\t=%p .kind=%d, .name=%s, .len=%d, .inner=%p\n", &typeHead, typeHead.kind, typeHead.name, typeHead.len, typeHead.inner);
	printf("typeHead.inner\t=%p .kind=%d, .name=%s, .len=%d, .inner=%p\n", typeHead.inner, typeHead.inner->kind, typeHead.inner->name, typeHead.inner->len, typeHead.inner->inner);
	return typeHead.inner->len + typeHead.inner->kind + typeHead.kind - 3;
}
