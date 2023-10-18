struct type_tag {
	char *name;
	int point;
	struct type_tag *inner;
};

struct identifier_tag { //40byte
	struct type_tag type; //24byte
	char *name; 					// 8byte
	int len; 							// 8byte
};


struct identifier_tag global_identifier;

int main(){
	global_identifier.name = "my name";
	global_identifier.len = 5;
	global_identifier.name = "my name";

	struct type_tag type;
	struct type_tag type_next;
	type_next.name = "char";
	type.inner = &type_next;
	global_identifier.type = type;
	/*
	add(global_identifier.len, global_identifier.len);
	*/
	// printf("%s\n", type.inner->name);
	printf("%s\n", "Hello");
	//printf("%s\n", global_identifier.type.inner->name);
	return 5;
}
