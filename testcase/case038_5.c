int x;

int global_reset(){
	x = 0;
	return 0;
}

int global_sum(int y, int z){
	return x + y + z;
}

int main(){
	int x;
	x = 3;
	global_reset();
	return global_sum(x, 2);
}

