int  print_add(int x, int y){
	printf("%d + %d = %d", x, y, x+y);
	return x+y;
}

int main(void){
	//void *void_var = &a;
	return print_add(2, 3);
}

