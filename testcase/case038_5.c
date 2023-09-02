int x;

int global_reset(){
	x = 0;
	return 0;
}

int global_sum(int y){
	return x + y;
}

int main(){
	int x;
	x = 3;
	global_reset();
	return global_sum(x);
}

