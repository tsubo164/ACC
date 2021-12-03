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

    /* TODO may need struct declaration or parse_context */
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

    /* conversion context */
    int is_sizeof_operand;
    int is_addressof_operand;
    int is_array_initializer;

    /* for initializer */
    struct data_type *init_type;
    const struct symbol *init_sym;
};

extern struct parser *new_parser(void);
extern void free_parser(struct parser *p);
extern struct ast_node *parse_text(struct parser *p, const char *text);

#endif /* _H */
