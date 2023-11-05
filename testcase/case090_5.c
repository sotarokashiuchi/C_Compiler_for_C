struct Token {
    int kind;
    int value_or_string_size;
    char *identifier_name_or_escaped_string_content;
    char *position;
};

struct Token *tokens_cursor;
struct Token tokens_start[5];

int is_reserved_then_handle(char *ptr, char *keyword, int keyword_len, int kind) {
    if (strncmp(ptr, keyword, keyword_len) != 0){
        return 0;
		}
		printf("%s\n", keyword);
    (tokens_cursor++)->kind = kind;
    return 1;
}

int main(void){
	tokens_cursor = tokens_start;
	is_reserved_then_handle("int main", "int", 3, 7630441);
	is_reserved_then_handle("main", "main", 4, 7630436);
	for(int i=0; i<5; i++){
		printf("tokens[%d], .kind=%x, .size=%d\n", i, tokens_start[i].kind, tokens_start[i].value_or_string_size);
	}
	return tokens_start[0].kind - tokens_start[1].kind;
}

