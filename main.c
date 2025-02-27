#include <stdio.h>
#include <stdlib.h>
#include <parser.h>
#include <lexer.h>
#include <codegen.h>

int main(void)
{
    token_t *l = lexer_parse(
            "int main(void)\n"
            "{\n"
            "   long int x = \"hi chat!\\n\";\n"
            "   long int a = \"hi discord!\\n\";\n"
            "   write(0, x, 10);\n"
            "   write(0, a, 13);\n"
            "   return 3;\n"
            "}"
    );

    struct statement_list *ast = ast_parse();
    FILE *f = fopen("out.S", "w");
    gen_asm(f, ast);

    fclose(f);

    return 0;
}
