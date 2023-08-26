#include <stdio.h>
#include <assert.h>

#define TESTCODE_BUF 10000
#define TESTLINE_BUF 1000

int read_file(char* buf, FILE *fp){
	char test_line[TESTCODE_BUF];
	char *buf_ptr = buf;
	char *test_line_ptr = NULL;
	while(fgets(test_line, TESTLINE_BUF, fp)){
		printf("test_line = %s\n", test_line);
		for(test_line_ptr=test_line; *test_line_ptr != '\0'; buf_ptr++, test_line_ptr++){
			*buf_ptr = *test_line_ptr;
		}
		printf("buf = %s\n", buf);
	}
	*buf_ptr = '\0';
	return 0;
}


int main(int argc, char **argv){
	FILE *fp = NULL;
	char test_code[TESTCODE_BUF];
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

		printf("Input:");
		read_file(test_code, fp);

		printf("********************************** [compile] **********************************\n");

		printf("********************************* [assemble] **********************************\n");

		printf("********************************* [execution] *********************************\n");

		return 0;
	} else {
    fprintf(stderr, "エラー:引数の個数が正しくありません\n");
    return 1;
  }

	printf("%s", argv[1]);

  /* 初期化 */
	/*
  char *debug = getenv("CC_DEBUG");
  debugEnabled = debug && !strcmp(debug, "1");
	user_input = argv[1];
  Identifier_t dummy = {NULL, NULL, 0, 0};
  identHead = &dummy;
	*/ 

}

