int main(){
	int x;
	x = 5;
	if(++x != 6){
		return 1;
	}
	if(x++ != 6){
	 	return 1;
	}
	if(--x != 6){
	 	return 1;
	}
	if(x-- != 6){
	 	return 1;
	}
	return x;
}

