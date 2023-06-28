sou@ThinkPadT14s:~/C_Compiler_for_C$ cc -std=c11 -g -c -I ./include -static -o main.o ./src/codegen.c 
In file included from ./src/codegen.c:2:
./include/codegen.h:8:17: error: unknown type name ‘Node_t’
    8 | extern void gen(Node_t *node);
      |                 ^~~~~~
./src/codegen.c:4:10: error: unknown type name ‘Node_t’
    4 | void gen(Node_t *node) {
      |          ^~~~~~
sou@ThinkPadT14s:~/C_Compiler_for_C$ cc -std=c11 -g -c -I ./include -static -o main.o ./src/codegen.c ./src/main.c ./src/parse.c ./src/tokenize.c 
cc: fatal error: cannot specify ‘-o’ with ‘-c’, ‘-S’ or ‘-E’ with multiple files
compilation terminated.
sou@ThinkPadT14s:~/C_Compiler_for_C$ cc -std=c11 -g -I ./include -static -o main.o ./src/codegen.c ./src/main.c ./src/parse.c ./src/tokenize.c 
In file included from ./src/codegen.c:2:
./include/codegen.h:8:17: error: unknown type name ‘Node_t’
    8 | extern void gen(Node_t *node);
      |                 ^~~~~~
./src/codegen.c:4:10: error: unknown type name ‘Node_t’
    4 | void gen(Node_t *node) {
      |          ^~~~~~
./src/parse.c: In function ‘primary’:
./src/parse.c:36:7: warning: implicit declaration of function ‘consume’ [-Wimplicit-function-declaration]
   36 |   if (consume('(')) {
      |       ^~~~~~~
./src/parse.c:38:5: warning: implicit declaration of function ‘expect’ [-Wimplicit-function-declaration]
   38 |     expect(')');
      |     ^~~~~~
./src/parse.c:43:23: warning: implicit declaration of function ‘expect_number’ [-Wimplicit-function-declaration]
   43 |   return new_node_num(expect_number());
      |                       ^~~~~~~~~~~~~
sou@ThinkPadT14s:~/C_Compiler_for_C$ cc -std=c11 -g -I ./include -static -o main.o ./src/codegen.c ./src/main.c ./src/parse.c ./src/tokenize.c 
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:6:3: error: redeclaration of enumerator ‘ND_ADD’
    6 |   ND_ADD,                               // +
      |   ^~~~~~
In file included from ./src/main.c:3:
./include/parse.h:6:3: note: previous definition of ‘ND_ADD’ with type ‘enum <anonymous>’
    6 |   ND_ADD,                               // +
      |   ^~~~~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:7:3: error: redeclaration of enumerator ‘ND_SUB’
    7 |   ND_SUB,                               // -
      |   ^~~~~~
In file included from ./src/main.c:3:
./include/parse.h:7:3: note: previous definition of ‘ND_SUB’ with type ‘enum <anonymous>’
    7 |   ND_SUB,                               // -
      |   ^~~~~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:8:3: error: redeclaration of enumerator ‘ND_MUL’
    8 |   ND_MUL,                               // *
      |   ^~~~~~
In file included from ./src/main.c:3:
./include/parse.h:8:3: note: previous definition of ‘ND_MUL’ with type ‘enum <anonymous>’
    8 |   ND_MUL,                               // *
      |   ^~~~~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:9:3: error: redeclaration of enumerator ‘ND_DIV’
    9 |   ND_DIV,                               // /
      |   ^~~~~~
In file included from ./src/main.c:3:
./include/parse.h:9:3: note: previous definition of ‘ND_DIV’ with type ‘enum <anonymous>’
    9 |   ND_DIV,                               // /
      |   ^~~~~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:10:3: error: redeclaration of enumerator ‘ND_NUM’
   10 |   ND_NUM,                               // 整数
      |   ^~~~~~
In file included from ./src/main.c:3:
./include/parse.h:10:3: note: previous definition of ‘ND_NUM’ with type ‘enum <anonymous>’
   10 |   ND_NUM,                               // 整数
      |   ^~~~~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:11:3: error: conflicting types for ‘NodeKind’; have ‘enum <anonymous>’
   11 | } NodeKind;
      |   ^~~~~~~~
In file included from ./src/main.c:3:
./include/parse.h:11:3: note: previous declaration of ‘NodeKind’ with type ‘NodeKind’
   11 | } NodeKind;
      |   ^~~~~~~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:14:16: error: redefinition of ‘struct Node’
   14 | typedef struct Node {
      |                ^~~~
In file included from ./src/main.c:3:
./include/parse.h:14:16: note: originally defined here
   14 | typedef struct Node {
      |                ^~~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:19:3: error: conflicting types for ‘Node_t’; have ‘struct Node’
   19 | } Node_t;
      |   ^~~~~~
In file included from ./src/main.c:3:
./include/parse.h:19:3: note: previous declaration of ‘Node_t’ with type ‘Node_t’ {aka ‘struct Node’}
   19 | } Node_t;
      |   ^~~~~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:23:16: error: conflicting types for ‘expr’; have ‘Node_t *(void)’ {aka ‘struct Node *(void)’}
   23 | extern Node_t *expr(void);
      |                ^~~~
In file included from ./src/main.c:3:
./include/parse.h:23:16: note: previous declaration of ‘expr’ with type ‘Node_t *(void)’ {aka ‘struct Node *(void)’}
   23 | extern Node_t *expr(void);
      |                ^~~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:27:16: error: conflicting types for ‘mul’; have ‘Node_t *(void)’ {aka ‘struct Node *(void)’}
   27 | extern Node_t *mul(void);
      |                ^~~
In file included from ./src/main.c:3:
./include/parse.h:27:16: note: previous declaration of ‘mul’ with type ‘Node_t *(void)’ {aka ‘struct Node *(void)’}
   27 | extern Node_t *mul(void);
      |                ^~~
In file included from ./include/codegen.h:4,
                 from ./src/main.c:4:
./include/parse.h:31:16: error: conflicting types for ‘primary’; have ‘Node_t *()’ {aka ‘struct Node *()’}
   31 | extern Node_t *primary();
      |                ^~~~~~~
In file included from ./src/main.c:3:
./include/parse.h:31:16: note: previous declaration of ‘primary’ with type ‘Node_t *()’ {aka ‘struct Node *()’}
   31 | extern Node_t *primary();
      |                ^~~~~~~
./src/parse.c: In function ‘primary’:
./src/parse.c:36:7: warning: implicit declaration of function ‘consume’ [-Wimplicit-function-declaration]
   36 |   if (consume('(')) {
      |       ^~~~~~~
./src/parse.c:38:5: warning: implicit declaration of function ‘expect’ [-Wimplicit-function-declaration]
   38 |     expect(')');
      |     ^~~~~~
./src/parse.c:43:23: warning: implicit declaration of function ‘expect_number’ [-Wimplicit-function-declaration]
   43 |   return new_node_num(expect_number());
      |                       ^~~~~~~~~~~~~
