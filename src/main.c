#include "common.h"
#include "tokenize.h"
#include "parse.h"
#include "codegen.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>

/* グローバル変数定義 */
char *user_input;
int debugEnabled;

char *read_file(char *path) {
	FILE *fp = fopen(path, "r");
	if(!fp){
		error("cannot open %s: %s", path, strerror(errno));
	}

	if(fseek(fp, 0, SEEK_END) == -1){
		error("%s: fseek %s", path, strerror(errno));
	}
	size_t size = ftell(fp);

	if(fseek(fp, 0, SEEK_SET) == -1){
		error("%s: fseek %s", path, strerror(errno));
	}

	char *buf = calloc(1, size +2);
	fread(buf, size, 1, fp);

	if(size == 0 || buf[size - 1] != '\n'){
		buf[size++] = '\n';
	}
	buf[size] = '\0';
	fclose(fp);
	return buf;
}

int main(int argc, char **argv){
	DEBUG_WRITE("\n");
	// 入力データの確認
	if(argc != 2){
		fprintf(stderr, "エラー:引数の個数が正しくありません\n");
		return 1;
	}

	/* 初期化 */
	char *debug = getenv("CC_DEBUG");
	debugEnabled = debug && !strcmp(debug, "1");
	user_input = read_file(argv[1]);
	// printf("%s", user_input);
	Identifier_t dummyIdentifier = {0, NULL, NULL, 0, 0};
	identHead = &dummyIdentifier;
	StringVector_t dummyStringVecotr = {NULL, 0, 0, NULL};
	stringHead = &dummyStringVecotr;

	/* トークナイズ */
	token = tokenize();
	DEBUG_WRITE("\033[35mcompleted tokenize\033[39m\n\n");

	/* パーサ */
	program();
	DEBUG_WRITE("\033[35mcompleted parse\033[39m\n\n");

	/* コード生成 */
	gens();
	DEBUG_WRITE("\033[35mcompleted codegen\033[39m\n\n");

	return 0;
}

void debug_write(char *fmt, ...){
	if(!debugEnabled){
		return;
	}
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

