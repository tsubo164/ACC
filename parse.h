#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "lexer.h"
#include "symbol.h"
#include "diagnostic.h"
#include "type.h"

#define TOKEN_BUFFER_SIZE 4

struct parser {
    struct lexer *lex;
    struct token tokbuf[TOKEN_BUFFER_SIZE];
    int head, curr;

    struct symbol_table *symtab;
    struct diagnostic *diag;
    struct data_type *func;

    /* error recovery */
    int is_panic_mode;

    /* conversion context */
    int is_sizeof_operand;
    int is_addressof_operand;
    int is_array_initializer;
};

extern struct parser *new_parser(void);
extern void free_parser(struct parser *p);
extern struct ast_node *parse_text(struct parser *p, const char *text,
        struct symbol_table *symtab, struct diagnostic *diag);

#endif /* _H */
