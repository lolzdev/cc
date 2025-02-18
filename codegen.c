#include <codegen.h>

extern struct literal_pair *literals_head;

static const char *call_regs[] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9"
};

static size_t str_count = 0;

void gen_literals(FILE *f)
{
    fprintf(f, ".data\n");
    while (literals_head != NULL) {
        fprintf(f, "str%d: .asciz \"%s\"\n", literals_head->label, literals_head->literal);

        literals_head = literals_head->next;
    }
}

void gen_block(FILE *f, struct block_member *block)
{
    while (block != NULL) {
        gen_statement(f, block->value);

        block = block->next;
    }
}

void gen_def(FILE *f, ast_statement_t *statement)
{
    // TODO: implement
}

void gen_mov(FILE *f, ast_node_t *expr, char *reg)
{
    if (expr == NULL) return;

    if (expr->ty == INTEGER) {
        fprintf(f, "mov $%d, %%%s\n", expr->expr.integer, reg);
    } else if (expr->ty == STRING_LIT) {
        fprintf(f, "mov $str%d, %%%s\n", expr->expr.string, reg);
    }
}

void gen_call(FILE *f, ast_statement_t *statement)
{
    for (int i=0; i < statement->statement.call.arg_count; i++) {
        gen_mov(f, statement->statement.call.args[i], call_regs[i]);
    }

    fprintf(f, "call %s\n", statement->statement.call.identifier);
}

void gen_function(FILE *f, ast_statement_t *statement)
{
    fprintf(f, "%s:\n", statement->statement.function.identifier);
    if (statement->statement.function.block != NULL) {
        gen_block(f, statement->statement.function.block);
    }
}

void gen_statement(FILE *f, ast_statement_t *statement)
{
    if (statement != NULL) {
        ast_type_t ty = statement->t;
        if (ty == FUNCTION) {
            gen_function(f, statement);
        } else if (ty == VAR_DEF) {
            gen_def(f, statement);
        } else if (ty == VAR_ASSIGN) {
            // assign variable
        } else if (ty == CALL) {
            gen_call(f, statement);
        }
    }
}

void gen_asm(FILE *f, struct statement_list *statements)
{
    gen_literals(f);

    fprintf(f, ".text\n.global main\n");

    while (statements != NULL) {
        gen_statement(f, statements->statement);    

        statements = statements->next;
    }
}
