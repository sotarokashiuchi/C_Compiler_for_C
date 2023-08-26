int main(){
	int x[10];
	x[0] = 5;
	x[1] = 8;
	int i;
	i = 0;
	if(x[i] < x[i+1]){
		x[i] = 1;
	}
	return x[0];
}
