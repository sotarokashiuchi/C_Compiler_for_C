//#include <stdio.h>
//#include <string.h>

int is_alnum(char c) {
    return strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", c) != 0;
}

int is_reserved_then_handle(char *ptr, int *ptr_i,  char *keyword, int keyword_len, int kind) {
	printf("   is_reserved_then_handle %.*s\n", keyword_len, keyword);
	if (strncmp(ptr, keyword, keyword_len) != 0){
		// keywordではない
		printf("   is_reserved_then_handle first if\n");
		return 0;
	}
	if (is_alnum(ptr[keyword_len])){
		// keywordではない
		printf("   is_reserved_then_handle second if\n");
		return 0;
	}
	// keywordである
	*ptr_i += keyword_len;
	return 1;
}

int main(void){
	int i=0;
	char *str = "int main (void) { return 5; }";
	if (is_reserved_then_handle(str + i, &i, "return", 6, 1)) {
		printf("return\n");
	} else if (is_reserved_then_handle(str + i, &i, "sizeof", 6, 2)) {
		printf("sizeof\n");
	} else if (is_reserved_then_handle(str + i, &i, "int", 3, 3)) {
		printf("int\n");
	} 
	return 5;
}
