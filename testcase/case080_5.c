//#include <string.h>
int is_alnum(char c) {
	return strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", c) != 0;
}

int main(void){
	if(is_alnum('_') != 0){
		return 5;
	} else {
		return 0;
	}
}
