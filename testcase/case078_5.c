struct Type {
	int kind;
	char *name;
	int len;
	struct Type *inner;
};

struct Type new_type(int kind, char *name, int len, struct Type oldType){
	printf("oldType\t\t=%p .kind=%d, .name=%s, .len=%d, .inner=%p\n", &oldType, oldType.kind, oldType.name, oldType.len, oldType.inner);
	oldType.kind = kind;
	oldType.name = name;
	oldType.len = len;
	printf("newType\t\t=%p .kind=%d, .name=%s, .len=%d, .inner=%p\n", &oldType, oldType.kind, oldType.name, oldType.len, oldType.inner);
	return oldType;
}

int main (void){
	struct Type typeHead;
	typeHead.kind = 1;
	typeHead.len = 4;
	typeHead.name = "name";
	typeHead = new_type(2, "names", 3, typeHead);
	printf("typeHead\t=%p .kind=%d, .name=%s, .len=%d, .inner=%p\n", &typeHead, typeHead.kind, typeHead.name, typeHead.len, typeHead.inner);
	return typeHead.len + typeHead.kind;
}

