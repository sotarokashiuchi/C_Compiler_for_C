
struct taype_tag {
	int x;
	int y;
	char z;
};

int main(){
	struct taype_tag A;
	A.x = 3;
	A.y = 2;

	struct taype_tag B[5];
	B[0].x = 5;
	for(int i=0; i<5; i++){
		B[i].x = i;
		B[i].y = i*2;
		B[i].z = i*3;
	}
	return A.x + B[3].y - B[4].y + B[1].x + B[1].z;
}


