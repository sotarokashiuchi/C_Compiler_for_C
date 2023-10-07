int main(){
	int x[10][5];
	if(sizeof(x) != 200){
		return 0;
	}
	if(sizeof(x[0]) != 20){
		printf("%d", sizeof(x[0]));
		printf("%d", sizeof(x[0][0]));
		return 1;
	}
	if(sizeof(int) != 4){
		return 2;
	}
	if(sizeof(int*) != 8){
		return 3;
	}
	if(sizeof(int[3][5]) != 60){
		return 4;
	}
	return 5;
}

