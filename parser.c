#include <parser.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern token_t *list;
extern token_t *head;

token_t *next(void)
{
    list = list->next;
    return list;
}

uint8_t check_consume(token_type_t ty)
{
    if (!list) return 0;
    if (list->ty == ty) {
        list = list->next;
        return 1;
    }

    return 0;
}

uint8_t check_next(token_type_t ty)
{
    if (!(list->next)) return 0;
    return list->next->ty == ty;
}

uint8_t check_token(token_type_t ty)
{
    if (!list) return 0;
    return list->ty == ty;
}

ast_node_t *ast_parse(void)
{
    list = head;
    list = list->next;
    ast_statement();
    return expression();
}

ast_statement_t *ast_function(void)
{
    ast_statement_t *fn = malloc(sizeof(ast_statement_t));
    fn->t = FUNCTION;
    fn->statement.function.ty = ast_type();
    if (!check_token(IDENTIFIER)) {
        // TODO: error
    }
    fn->statement.function.identifier = list->lexeme;
    next();
    check_consume(L_PAREN);
    token_t *tmp = list;
    expr_type_t arg_type = ast_type();
    printf("a: %d\n", arg_type);
    if ((arg_type & TYPE_MASK) == VOID_T) {
        if (arg_type & TYPE_POINTER) {
            printf("a1: %d\n", arg_type);
            goto check_args;
        } else {
            check_consume(R_PAREN);
            fn->statement.function.arg_count = 0;
        }
    } else if (!check_consume(R_PAREN)) {
check_args:
        list = tmp;
        size_t arg_count = 0;
        while (!check_token(R_PAREN)) {
            if (check_consume(COMMA)) {
                arg_count += 1;
            } else {
                next();
            }
        }
        arg_count += 1;
        fn->statement.function.arg_count = arg_count;
        fn->statement.function.args = (struct arg *) malloc(sizeof(struct arg) * arg_count);
        struct arg *args = fn->statement.function.args;

        for (size_t i=0; !check_consume(R_PAREN); i++) {
            args[i].ty = ast_type();
            if (!check_token(IDENTIFIER)) {
                args[i].identifier = NULL;
            } else {
                args[i].identifier = list->lexeme;
            }

            check_consume(COMMA);
        }
    } else {
        fn->statement.function.arg_count = 0;
    }

    printf("t: %d\n", fn->statement.function.arg_count);
    if (check_consume(SEMICOLON)) {
        fn->statement.function.block = NULL;
    } else if (check_token(L_CURLY)) {
        fn->statement.function.block = ast_block();
    } else {
        // TODO: error
    }

    if (fn->statement.function.block != NULL && fn->statement.function.block->value != NULL) {
        printf("%d\n", fn->statement.function.block->value->t);
    }

    return fn;
}

struct block_member *ast_block(void)
{
    struct block_member *block = NULL;
    struct block_member *head = block;

    if (!check_consume(L_CURLY)) {
        // TODO: error
    }

    if (check_consume(R_CURLY)) {
        return NULL;
    }

    while (!check_token(R_CURLY)) {
        ast_statement_t *statement = ast_statement();
        if (block == NULL) {
            block = malloc(sizeof(struct block_member));
        }

        if (head == NULL) {
            head = block;
        }

        block->value = statement;

        block->next = malloc(sizeof(struct block_member));
        block = block->next;
        block->value = NULL;
    }

    return head;
}

ast_statement_t *ast_statement(void)
{
    token_t *tmp = list;
    switch (list->ty) {
        case LONG:
        case SHORT:
        case UNSIGNED:
        case CHAR:
        case INT:
        case FLOAT:
        case DOUBLE:
        case VOID: {
            tmp = list;
            ast_type();
            check_consume(IDENTIFIER);
            if (check_token(ASSIGN)) {
                list = tmp;
                // TODO: variable
            } else if (check_token(L_PAREN)) {
                list = tmp;
                return ast_function();
            }
        }
        default:
            break;
    }
}

ast_node_t *expression(void)
{
    ast_node_t *t = term();

    while (check_token(PLUS) || check_token(MINUS)) {
        ast_node_t *b = malloc(sizeof(ast_node_t));
        b->ty = BINARY;
        b->expr.binary.op = check_token(PLUS) ? OP_PLUS : OP_MINUS;
        b->expr.binary.left = t;
        next();
        b->expr.binary.right = term();

        t = b;
    }

    return t;
}

ast_node_t *term(void)
{
    ast_node_t *t = factor();

    while (check_token(STAR) || check_token(SLASH)) {
        ast_node_t *b = malloc(sizeof(ast_node_t));
        b->ty = BINARY;
        b->expr.binary.op = check_token(STAR) ? OP_STAR : OP_SLASH;
        b->expr.binary.left = t;
        next();
        b->expr.binary.right = factor();

        t = b;
    }

    return t;
}

ast_node_t *factor(void)
{
    if (check_token(NUMBER)) {
        ast_node_t *node = malloc(sizeof(ast_node_t));
        node->expr.integer = atoi(list->lexeme);
        node->ty = INTEGER;
        next();

        return node;
    }

    return NULL;
}

expr_type_t ast_type(void)
{
    expr_type_t res;

    if (check_token(STRUCT)) {
        char *s_name = next()->lexeme;
        char *s_ty = aligned_alloc(0x1000, strlen(s_name)+1);
        strcpy(s_ty, s_name);
        next();
        res = ((size_t)s_ty) | (STRUC & 0xfff);
    } else if (check_consume(LONG)) {
        check_consume(INT);
        res = INT64;
    } else if (check_consume(SHORT)) {
        check_consume(INT);
        res = INT16;
    } else if (check_consume(UNSIGNED)) {
        expr_type_t t = ast_type();
        switch (t) {
            case INT8:
                res = UINT8;
                break;
            case INT16:
                res = UINT16;
                break;
            case INT32:
                res = UINT32;
                break;
            case INT64:
                res = UINT64;
                break;
            default:
                // TODO: error
                break;
        }
    } else if (check_consume(CHAR)) {
        res = INT8;
    } else if (check_consume(INT)) {
        res = INT32;
    } else if (check_consume(VOID)) {
        res = VOID_T;
    }

    if (check_consume(STAR)) {
        res |= TYPE_POINTER;
    }

    return res;
}

void ast_walk(ast_node_t *ast)
{
    if ((ast->ty & 0xfff) == INTEGER) {
        printf("n: %ld\n", ast->expr.integer);
    } else if ((ast->ty) == BINARY) {
        printf("(");
        ast_walk(ast->expr.binary.left);
        printf(" + ");
        ast_walk(ast->expr.binary.right);
        printf(")\n");
    }
}
