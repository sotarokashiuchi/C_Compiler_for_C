char* genRegFromSize(int n, int sz){
	if (sz == 8){
		return &"rdi\0rsi\0rdx\0rcx\0r8 \0r9"[4 * n];
	}else if (sz == 4){
		return &"edi\0esi\0edx\0ecx\0r8d\0r9d"[4 * n];
	}else if (sz == 1){
		return &"dil\0sil\0dl \0cl \0r8b\0r9b"[4 * n];
	}
	printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
	exit(1);
}

int main(void){
	char *str;
	str = genRegFromSize(2, 8);
	printf("%s\n", str);
	return 5;
}
