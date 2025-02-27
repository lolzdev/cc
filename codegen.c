#include <codegen.h>
#include <stddef.h>
#include <stdlib.h>
#include <trie.h>

extern struct literal_pair *literals_head;

static const char *call_regs[] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9"
};

static struct scope *scope_stack = NULL;

struct trie *variables = NULL;
static size_t str_count = 0;
static size_t base_offset = 0;

void push_scope(size_t base, size_t pointer)
{
    struct scope *scope = (struct scope *) malloc(sizeof(struct scope));
    scope->stack_base = base;
    scope->stack_pointer = pointer;

    scope->prev = scope_stack;
    scope_stack->next = scope;
    scope_stack = scope;
}

struct scope *pop_scope()
{
    struct scope *tmp = scope_stack;
    scope_stack = scope_stack->prev;
    tmp->prev = NULL;
    return tmp;
}

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

void gen_assignment(FILE *f, ast_statement_t *assignment)
{
    gen_mov(f, assignment->statement.var_assign.value, "r10");
    fprintf(f, "mov %%r10, -%x(%rbp)\n", trie_get(variables, assignment->statement.var_assign.identifier));
}

void gen_def(FILE *f, ast_statement_t *statement)
{
    trie_insert(variables, statement->statement.var_def.identifier, base_offset);
    base_offset += ast_type_size(statement->statement.var_def.ty);

    if (statement->statement.var_def.assignment != NULL) {
    printf("test\n");
        gen_assignment(f, statement->statement.var_def.assignment);
    }
}

void gen_mov(FILE *f, ast_node_t *expr, char *reg)
{
    if (expr == NULL) return;

    if (expr->ty == INTEGER) {
        fprintf(f, "mov $%d, %%%s\n", expr->expr.integer, reg);
    } else if (expr->ty == STRING_LIT) {
        fprintf(f, "mov $str%d, %%%s\n", expr->expr.string, reg);
    } else if (expr->ty == ID) {
        fprintf(f, "mov -0x%x(%%rbp), %%%s\n", trie_get(variables, expr->expr.identifier), reg);
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
    variables = (struct trie *) malloc(sizeof(struct trie));

    fprintf(f, "%s:\n", statement->statement.function.identifier);
    fprintf(f, "push %%rbp\n");
    fprintf(f, "mov %rsp, %rbp\n");
    if (statement->statement.function.block != NULL) {
        fprintf(f, "sub $0x%x, %%rsp\n", statement->statement.function.block->stack_size);
        gen_block(f, statement->statement.function.block);
    }

    free(variables);
}

void gen_return(FILE *f, ast_statement_t *statement)
{
    gen_mov(f, statement->statement.ret.value, "rax");
    fprintf(f, "leave\nret\n");
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
            gen_assignment(f, statement);
        } else if (ty == CALL) {
            gen_call(f, statement);
        } else if (ty == RETURN_STATEMENT) {
            gen_return(f, statement);
        }
    }
}

void gen_asm(FILE *f, struct statement_list *statements)
{
    gen_literals(f);

    scope_stack = (struct scope *) malloc(sizeof(struct scope));

    fprintf(f, ".text\n.global main\n");

    while (statements != NULL) {
        gen_statement(f, statements->statement);    

        statements = statements->next;
    }

    while (scope_stack) {
        struct scope *tmp = scope_stack->prev;
        scope_stack = scope_stack->prev;
        free(tmp);
    }
}
