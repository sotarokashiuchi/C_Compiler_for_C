#include <stdio.h>
#include <assert.h>

#define TESTCODE_BUF 10000
#define TESTLINE_BUF 1000



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

