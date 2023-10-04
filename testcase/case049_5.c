int main(){
	int x;
	int **y;
	int str[5];
	char t;
	x = 5;
	printf("sizeof(x) = %d\n", sizeof(x));
	printf("sizeof(&x) = %d\n", sizeof(&x));
	printf("sizeof(y) = %d\n", sizeof(y));
	printf("sizeof(*y) = %d\n", sizeof(*y));
	printf("sizeof(**y) = %d\n", sizeof(**y));
	// printf("sizeof(str) = %d\n", sizeof(str));
	printf("sizeof(t) = %d\n", sizeof(t));
	// printf("sizeof(int) = %d\n", sizeof(int));
	return 5;
}

