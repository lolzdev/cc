#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <lexer.h>

int main(void)
{
    token_t *l = lexer_parse("int f(void) { int x(void) {} }");

    ast_node_t *ast = ast_parse();
    //ast_walk(ast);

    return 0;
}
