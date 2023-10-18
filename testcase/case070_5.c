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


struct identifier_tag global_identifier;

int main(){
	struct identifier_tag identifier[5];
	struct type_tag type[5];
	global_identifier.type = type[0];
	global_identifier.name = "my name";
	global_identifier.len = 5;
	for(int i=0; i<5; i++){
		type[i].len=i;
		type[i].name="name";
		printf("type[%d].name = %s .len = %d\n", i, type[i].name, type[i].len);
	}
	for(int i=0; i<5; i++){
		identifier[i].type=type[i];
		identifier[i].name = "identName";
		identifier[i].len = i;
		printf("identifier[%d].name = %s .len = %d\n", i, identifier[i].name, identifier[i].len);
	}
	global_identifier.type = type[2];
	printf("%s\n", global_identifier.type.inner->name);
	return 5;
}
