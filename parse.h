#ifndef PARSE_H
#define PARSE_H

#include "lexer.h"
#include "symbol.h"

#define TOKEN_BUFFER_SIZE 2

struct parser {
    struct lexer lex;
    struct token tokbuf[TOKEN_BUFFER_SIZE];
    int head, curr;

    struct symbol_table symtbl;

    long error_pos;
    const char *error_msg;
};

enum ast_node_kind {
    NOD_LIST,
    NOD_STMT,
    NOD_EXT,  /* to extend child nodes */
    NOD_IF,
    NOD_RETURN,
    NOD_WHILE,
    NOD_ASSIGN,
    NOD_VAR,
    NOD_VAR_DEF,
    NOD_ADDR,
    NOD_DEREF,
    NOD_CALL,
    NOD_FUNC_DEF,
    NOD_ARG,
    NOD_PARAM,
    NOD_NUM,
    NOD_ADD,
    NOD_SUB,
    NOD_MUL,
    NOD_DIV,
    NOD_LT,
    NOD_GT,
    NOD_LE,
    NOD_GE,
    NOD_EQ,
    NOD_NE
};

struct ast_node {
    enum ast_node_kind kind;
    struct ast_node *l;
    struct ast_node *r;
    union {
        int ival;
        const struct symbol *sym;
    } data;
};

extern void parser_init(struct parser *p);

extern struct ast_node *parse(struct parser *p);

#endif /* _H */
