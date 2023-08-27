int main(){
	int x;
	int result;
	result = 0;

	x = 3 == 3;
	if (x){
		result = 1;
	}

	x = 3 != 3;
	if (x){
		result = 2;
	}
	
	return result;
}
