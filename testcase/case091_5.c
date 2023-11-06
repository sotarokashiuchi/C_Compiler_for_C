// #include <stdio.h>
 char *AddSubMulDivAssign_rdi_into_rax(int kind) {
    if (kind == 1)
        return "  add rax,rdi\n";
    else if (kind == 2)
        return "  sub rax,rdi\n";
    else if (kind == 3)
        return "  imul rax,rdi\n";
    else if (kind == 4)
        return "  cqo\n  idiv rdi\n";
    return 0;
}

int main(void){
	char array[10];
	printf("%p\n", array);
	printf("%p\n", &array[0]);
	printf("%s\n", AddSubMulDivAssign_rdi_into_rax(2));
	printf("%d\n", sizeof(array));
	return 5;
}
