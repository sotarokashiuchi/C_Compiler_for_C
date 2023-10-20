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
	struct identifier_tag identifier;
	struct type_tag type;
	type.len = 5;
	identifier.type = type;
	printf("%d\n", identifier.type.len);
	return identifier.type.len;
}
