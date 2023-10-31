int is_alnum(char c) {
    return strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", c) != 0;
}

int is_reserved_then_handle(char *ptr, int *ptr_i,  char *keyword, int keyword_len, int kind) {
	if (strncmp(ptr, keyword, keyword_len) != 0){
		// keywordではない
		return 0;
	}
	if (is_alnum(ptr[keyword_len])){
		// keywordではない
		return 0;
	}
	// keywordである
	*ptr_i += keyword_len;
	return 1;
}

int main(void){
	int i=0;
	char *str = "int return return sizeof int ";
	while(str[i]){
		if (is_reserved_then_handle(str + i, &i, "return", 6, 1)) {
			printf("return\n");
		} else if (is_reserved_then_handle(str + i, &i, "sizeof", 6, 2)) {
			printf("sizeof\n");
		} else if (is_reserved_then_handle(str + i, &i, "int", 3, 3)) {
			printf("int\n");
		} else if (str[i] == ' '){
			printf("blank\n");
			i++;
		}
	}
	return 5;
}
