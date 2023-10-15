struct taype_tag {
	int x;
	int y;
	char z;
};

int change(struct taype_tag *B){
	printf("B = %p\n", B);
	(*B).x = 3;
	(*B).y = 2;
	// B->x = 3;
	// B->y = 2;
	return 0;
}

int main(){
	struct taype_tag A;
	printf("A = %p\n", &A);
	change(&A);
	return A.x + A.y;
}


