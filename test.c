#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TESTCODE_BUF 10000
#define TESTLINE_BUF 1000

int read_file(char* buf, FILE *fp){
	char test_line[TESTCODE_BUF];
	char *buf_ptr = buf;
	char *test_line_ptr = NULL;
	while(fgets(test_line, TESTLINE_BUF, fp)){
		for(test_line_ptr=test_line; *test_line_ptr != '\0'; buf_ptr++, test_line_ptr++){
			*buf_ptr = *test_line_ptr;
		}
	}
	*buf_ptr = '\0';
	return 0;
}


int main(int argc, char **argv){
	FILE *fp = NULL;
	char test_code[TESTCODE_BUF];
	char command[TESTCODE_BUF+255];
	char file_name[255];
	sprintf(file_name, "%s%s", "testcase/", argv[1]);

	// 入力データの確認
  if(argc == 1){
		// list mode
		return 0;
	} else if(argc == 2){
		// debug mode
		printf("******************************** [information] ********************************\n");
		printf("FileName:%s\n", file_name);
		fp = fopen(file_name, "r");
		assert(fp != NULL && "failed to open file");
		printf("FieOpen:OK\n");

		read_file(test_code, fp);
		printf("Input:\n%s\n", test_code);

		printf("********************************** [compile] **********************************\n");
		sprintf(command, "CC_DEBUG=1 ./9cc \"%s\" > tmp.s", test_code);
		system(command);

		printf("********************************* [assemble] **********************************\n");
		system("cc -o link.o -c ./src/link.c");
		system("cc -o tmp.o -c tmp.s");
		system("cc -o tmp tmp.o link.o");

		printf("********************************* [execution] *********************************\n");
		system("./tmp");;

		return 0;
	} else {
    fprintf(stderr, "エラー:引数の個数が正しくありません\n");
    return 1;
  }
}

