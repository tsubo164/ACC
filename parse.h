#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "lexer.h"
#include "symbol.h"
#include "message.h"
#include "type.h"

#define TOKEN_BUFFER_SIZE 4

struct parser {
    struct lexer lex;
    struct token tokbuf[TOKEN_BUFFER_SIZE];
    int head, curr;

    struct symbol_table *symtab;
    struct message_list *msg;

    /* TODO may need struct declaration */
    /* declaration context */
    int decl_kind;
    const char * decl_ident;
    struct data_type *decl_type;

    /* for enum */
    int enum_value;
    /* for function */
    struct symbol *func_sym;

    int is_typedef;
    int is_extern;
    int is_static;
    int is_const;
    int is_unsigned;
    int is_panic_mode;

    /* for initializer */
    struct data_type *init_type;
    const struct symbol *init_sym;
};

extern struct parser *new_parser();
extern void free_parser(struct parser *p);
extern struct ast_node *parse(struct parser *p);

#endif /* _H */
