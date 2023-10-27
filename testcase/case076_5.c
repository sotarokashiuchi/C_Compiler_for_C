struct Type {
	int kind;
	char *name;
	int len;
	struct Type *inner;
};

int main (void){
	struct Type type;
	struct Type newType;
	type.kind = 3;
	type.len = 2;
	type.name = "name";
	newType = type;
	printf("type\t=%p .kind=%d, .name=%s, .len=%d, .inner=%p\n", &type, type.kind, type.name, type.len, type.inner);
	printf("newType\t=%p .kind=%d, .name=%s, .len=%d, .inner=%p\n", &newType, newType.kind, newType.name, newType.len, newType.inner);
	return newType.kind + newType.len;
}

