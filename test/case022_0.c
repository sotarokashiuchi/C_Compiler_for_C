int main(){
	int i;
	int j;
	i=0;
	j=0;
	while(i<5){
		while(j<5){
			intPrint(i*j);
			j=j+1;
		}
		i=i+1;
	}
	return 0;
}
