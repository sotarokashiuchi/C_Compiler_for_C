int main(){
	int *p = NULL;
	int x = 5;
	if(p == NULL){
		printf("if");
		p = &x;
	}
	return *p;
}

