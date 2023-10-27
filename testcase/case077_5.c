
struct Type {
	int kind;
	char *name;
	int len;
	struct Type *inner;
};

void print_type(struct Type type){
	printf("type\t=%p .kind=%d, .name=%s, .len=%d\n", &type, type.kind, type.name, type.len);
}


int main (void){
	struct Type currentType;
	currentType.kind = 0;
	currentType.len = 1;
	currentType.name = "name";
	//printf("currentType\t=%p .kind=%d, .name=%s, .len=%d\n", &currentType, currentType.kind, currentType.name, currentType.len);
	print_type(currentType);
	return 5;
}

