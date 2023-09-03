#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

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

int code_info(char* buf, int* expect_status, char* file_name){
	char *p = NULL;

	// file name
	sprintf(buf, "%s%s", "testcase/", file_name);

	// expect status
	for(p = file_name; *p != '_' && *p != '\0'; p++);
	assert(*p == '_' && "filed to catch expect status");
	*expect_status = atoi(++p);
	// printf("expect status = %d\n", *expect_status);
	return 0;
}

int is_successful(int expect_statas, int *status){
	FILE *fp = fopen("tmp_status", "r");
	if(fp==NULL){
		return 1;
	}
	fscanf(fp, "%d", status);
	if(*status == expect_statas){
		system("rm tmp_status");
		return 0;
	} else {
		system("rm tmp_status");
		return 1;
	}
}

int main(int argc, char **argv){
	FILE *fp = NULL;
	char test_code[TESTCODE_BUF];
	char command[TESTCODE_BUF+255];
	char file_name[255];
	int expect_status;
	int status;
	int all_status = 0;

	// switch test mode
  if(argc == 1){
		// list mode
		DIR *dir = NULL;
		struct dirent *dp =NULL;
		int case_index = 0;

		for(case_index=1 ; ; case_index++){
			dir = opendir("./testcase");
			assert(dir != NULL && "failed to open dir");
			sprintf(file_name, "case%03d", case_index);

			while((dp = readdir(dir)) != NULL){
				if(strncmp(dp->d_name, file_name, 7)){
					continue;
				}

				printf("%s\t:", dp->d_name);

				code_info(file_name, &expect_status, dp->d_name);
				fp = fopen(file_name, "r");
				assert(fp != NULL && "failed to open file");

				read_file(test_code, fp);
				sprintf(command, "CC_DEBUG=0 ./9cc \"%s\" > tmp.s", test_code);
				system(command);
				system("cc -o link.o -c ./testcase/link.c");
				system("cc -o tmp.o -c tmp.s");
				system("cc -o tmp tmp.o link.o");
				system("./tmp ; echo $? > tmp_status");

				if(is_successful(expect_status, &status) == 0){
					printf("\e[42mOK \e[0m");
				} else {
					printf("\e[41mERR\e[0m");
					all_status++;
				}

				printf("\n");
				//printf("%s", test_code);

				break;
			}
			if(dp == NULL){
				// not find out the file
				break;
			}
		}

		printf("failed : %d\n", all_status);
		return all_status;
	} else if(argc == 2){
		// debug mode
		printf("******************************** [information] ********************************\n");
		code_info(file_name, &expect_status, argv[1]);
		printf("FileName:%s\n", file_name);
		fp = fopen(file_name, "r");
		assert(fp != NULL && "failed to open file");
		printf("FieOpen:OK\n");

		read_file(test_code, fp);
		printf("Input:\n%s\n", test_code);
		printf("expect status:%d\n", expect_status);

		printf("********************************** [compile] **********************************\n");
		sprintf(command, "CC_DEBUG=1 ./9cc \"%s\" > tmp.s", test_code);
		system(command);

		printf("********************************* [assemble] **********************************\n");
		system("cc -o link.o -c ./testcase/link.c");
		system("cc -o tmp.o -c tmp.s");
		system("cc -o tmp tmp.o link.o");

		printf("********************************* [execution] *********************************\n");
		system("./tmp ; echo $? > tmp_status");

		printf("Input:\n%s\n", test_code);
		printf("expect status:%d\n", expect_status);

		if(is_successful(expect_status, &status) == 0){
			printf("\e[42mSUCCESS \e[0m");
		} else {
			printf("\e[41mERROR \e[0m");
			printf("expect %d but -> %d\n", expect_status, status);
		}

		return 0;
	} else {
    fprintf(stderr, "エラー:引数の個数が正しくありません\n");
    return -1;
  }
}

