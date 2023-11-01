//#include <stdio.h>
//#include <string.h>
int is_alnum(char c) {
    return strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", c) != 0;
}

int is_reserved_then_handle(char *ptr, int *ptr_i,  char *keyword, int keyword_len, int kind) {
	if (strncmp(ptr, keyword, keyword_len) != 0){
		return 0;
	}
	if (is_alnum(ptr[keyword_len])){
		return 0;
	}
	*ptr_i += keyword_len;
	return 1;
}

int gen(int x){
	return x+1;
}

int main(void){
	int i=0;
	char *str = "int return return sizeof int ";
	return printf("%04d\n", is_reserved_then_handle(str + i, &i, "return", 6, gen(0)));
}
