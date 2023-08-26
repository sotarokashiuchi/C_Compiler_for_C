int main(){
  int i;
  int j;
  int x[10];
  for(i=0; i<9; i=i+1){
	  x[i] = 8-i;
  }

  int tmp;
  int flags;
  flags = 0;
  for(i=0; i<8; i=i+1){
    for(j=i+1; j<9; j=j+1){
      if(x[i] > x[j]){
        tmp = x[j];
        x[j] = x[i];
        x[i] = tmp;
      }
    }
  }
  x[9] = 99;
  sortPrint(x);
  if(x[0] != 0){
	  flags = 1;
  }
  if(x[1] != 1){
	  flags = 1;
  }
  if(x[2] != 2){
	  flags = 1;
  }
  if(x[3] != 3){
	  flags = 1;
  }
  return flags;
}
