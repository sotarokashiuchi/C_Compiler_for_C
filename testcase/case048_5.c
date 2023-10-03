int main(){
	int x;
	x = 5;
	if(++x != 6){
		return 1;
	}
	printf("%d", x);
	if(x++ != 6){
	 	return 1;
	}
	return 5;
}

