struct Type {
	int kind;
	char *name;
	int len;
	struct Type *inner;
};

// プロトタイプ宣言
struct Type *new_type();

int main (void){
	struct Type *typeHead;
	typeHead = new_type(3, "names", 4);
	return typeHead->len + typeHead->kind - 2;
}

struct Type *new_type(int kind, char *name, int len){
	struct Type *newType = malloc(sizeof(struct Type));
	newType->kind = kind;
	newType->name = name;
	newType->len = len;
	return newType;
}

