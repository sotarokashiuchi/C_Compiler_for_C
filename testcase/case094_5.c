int main(void){
	int x;
	x = 5;
	switch(x){
		case 0:
			x = 0;
			printf("x = 0\n");
		case 2+4:
			x = 0;
			printf("x = 0\n");
		case 8:
			x = 0;
			printf("x = 0\n");
		default:
			x = 10;
			printf("x = 10\n");
	}
	return x-5;
}

