struct type_tag {
	struct type_tag *inner;
	char *name;
	int len;
};

struct identifier_tag { //40byte // offset
	struct type_tag type; //24byte // 0
	char *name; 					// 8byte // 24
	int len; 							// 8byte // 32
};


int main(){
	struct identifier_tag identifier;
	struct type_tag type;
	type.name = "My name";
	type.len = 7;
	type.inner = NULL;
	identifier.type = type;
	identifier.name = "Identifier Name";
	identifier.len = 15;
	printf("type .name = %s, .len = %d\n", type.name, type.len);
	printf("identifier .name = %s, .len = %d\n", identifier.name, identifier.len);
	printf("identifier.type .name = %s, .len = %d\n", identifier.type.name, identifier.type.len);
	return 5;
}
