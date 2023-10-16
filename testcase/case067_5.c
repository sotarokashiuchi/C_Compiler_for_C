struct A {
	int x;
	int y;
};

void  print_add(int x, int y){
	printf("%d + %d = %d", x, y, x+y);
}

int main(void){
	struct A a;
	void *voidVar = &a;
	printf("%p, %p\n", voidVar, &a);
	print_add(2, 3);
	return 5;
}

