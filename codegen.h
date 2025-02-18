#ifndef CODEGEN_H
#define CODEGEN_H

#include <parser.h>
#include <stdio.h>

void gen_function(FILE *f, ast_statement_t *statement);
void gen_block(FILE *f, struct block_member *block);
void gen_statement(FILE *f, ast_statement_t *statement);
void gen_asm(FILE *f, struct statement_list* statements);

#endif
