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
            "   write(0, \"hello world!\\n\", 13);\n"
            "   write(0, \"hello world again!\\n\", 19);\n"
            "}"
    );

    struct statement_list *ast = ast_parse();
    FILE *f = fopen("out.S", "w");
    gen_asm(f, ast);

    fclose(f);

    return 0;
}
