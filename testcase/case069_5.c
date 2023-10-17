struct type_tag {
	char *name;
	int point;
	struct type_tag *inner;
};

struct identifier_tag {
	struct type_tag type;
	int *name;
};

int main(){
	struct type_tag type;
	struct type_tag type_next;
	type_next.name = "char";
	type.inner = &type_next;
	printf("%s\n", type.inner->name);
	return 5;
}


