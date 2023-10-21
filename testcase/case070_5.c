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
		printf(".l=%d", type[0].len);
		printf(".l=%d", type[1].len);
		printf(".l=%d", type[2].len);
		printf(".l=%d", type[3].len);
		printf(".l=%d", type[4].len);
		printf("type[%d].name = %s .len = %d\n", i, type[i].name, type[i].len);
	}
	for(int i=0; i<5; i++){
		identifier[i].type=type[i];
		identifier[i].name = "identName";
		identifier[i].len = i;
		//printf(".l=%d", identifier[0].type.len);
		//printf(".l=%d", identifier[1].type.len);
		//printf(".l=%d", identifier[2].type.len);
		//printf(".l=%d", identifier[3].type.len);
		//printf(".l=%d", identifier[4].type.len);
		//printf("identifier[%d].name = %s .len = %d\n", i, identifier[i].name, identifier[i].len);
	}
	/*
	for(int i=0; i<5; i++){
		;
		printf("type[%d].name = %s .len = %d", i, identifier[i].type.name, identifier[i].type.len);
		printf("identifier[%d].name = %s .len = %d \n", i, identifier[i].name, identifier[i].len);
	}
	*/
	global_identifier.type = type[2];
	printf("%s\n", global_identifier.type.name);
	return 5;
}
