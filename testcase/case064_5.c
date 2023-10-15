struct taype_tag {
	int x;
	int y;
	char z;
};

int change(struct taype_tag *X){
	printf("X = %p\n", X);
	(*X).x = 1;
	(*X).y = 2;
	return 0;
}

int main(){
	struct taype_tag A;
	struct taype_tag B;
	printf("A = %p\n", &A);
	printf("B = %p\n", &B);
	change(&A);
	change(&B);
	return A.x + A.y + B.x + B.y -1;
}


