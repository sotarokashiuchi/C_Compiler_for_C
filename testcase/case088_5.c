struct Type {
	int kind;
	char *name;
	int len;
	struct Type *inner;
};

// プロトタイプ宣言
struct Type new_type();

int main (void){
	struct Type typeHead;
	typeHead.kind = 1;
	typeHead.len = 2;
	typeHead.name = "name";
	typeHead = new_type(3, "names", 4, &typeHead);
	return typeHead.inner->len + typeHead.inner->kind + typeHead.kind - 3;
}

struct Type new_type(int kind, char *name, int len, struct Type* typeHead){
	struct Type *newType = malloc(sizeof(struct Type));
	newType->kind = kind;
	newType->name = name;
	newType->len = len;
	typeHead->inner = newType;
	return *typeHead;
}

