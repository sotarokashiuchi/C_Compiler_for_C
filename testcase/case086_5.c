void panic( char *msg) {
    printf("!!!!!!!!!!!!!!!!!!!!!!!!! compile error !!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("! %s", msg);
    exit(1);
}


int main(int argc, char **argv) {
	printf("Point1\n");
	printf(" 	argc=%d\n", argc);
	if (argc != 1)
		panic("incorrect cmd line arg\n");
	printf(" 	argc=%d\n", argc);
	return 5;
}
