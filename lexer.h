#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "string_table.h"

enum token_kind {
    TOK_UNKNOWN = -1,
    TOK_END_OF_ASCII = 127,
    TOK_NUM,
    TOK_IDENT,
    TOK_STRING_LITERAL,
    /* keywords */
    TOK_ELSE,
    TOK_IF,
    TOK_RETURN,
    TOK_STRUCT,
    TOK_WHILE,
    /* types */
    TOK_CHAR,
    TOK_INT,
    /* bin op */
    TOK_LE,
    TOK_GE,
    TOK_EQ,
    TOK_NE,
    TOK_EOF
};

struct token {
    int kind;
    /* TODO rename this member */
    int value;
    const char *text;

    long file_pos;
};

struct lexer {
    FILE *file;
    long file_pos;
    struct string_table *strtab;
};

extern void token_init(struct token *tok);
extern long token_file_pos(const struct token *tok);

extern void lexer_init(struct lexer *lex);

extern struct lexer *new_lexer(FILE *fp, struct string_table *strtab);
extern void free_lexer(struct lexer *lex);

extern enum token_kind lex_get_token(struct lexer *l, struct token *tok);

#endif /* _H */
