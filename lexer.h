#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "string_table.h"

struct position {
    int x, y;
    const char *filename;
};

enum token_kind {
    TOK_UNKNOWN = -1,
    TOK_END_OF_ASCII = 127,
    TOK_NUM,
    TOK_IDENT,
    TOK_STRING_LITERAL,
    /* keywords */
    TOK_DO,
    TOK_ELSE,
    TOK_ENUM,
    TOK_FOR,
    TOK_IF,
    TOK_RETURN,
    TOK_STRUCT,
    TOK_WHILE,
    /* types */
    TOK_CHAR,
    TOK_INT,
    /* unary op */
    TOK_INC,
    TOK_DEC,
    /* bin op */
    TOK_LE,
    TOK_GE,
    TOK_EQ,
    TOK_NE,
    /* assignment op */
    TOK_ADD_ASSIGN,
    TOK_SUB_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    /* ---- */
    TOK_EOF
};

struct token {
    int kind;
    /* TODO rename this member */
    int value;
    const char *text;

    struct position pos;

    long file_pos;
};

struct lexer {
    FILE *file;
    long file_pos;
    struct string_table *strtab;

    struct position pos;
    int currc;
    int prevc;
    int prevx;
};

extern void token_init(struct token *tok);
extern long token_file_pos(const struct token *tok);

extern void lexer_init(struct lexer *lex);

extern struct lexer *new_lexer(FILE *fp, struct string_table *strtab);
extern void free_lexer(struct lexer *lex);

extern enum token_kind lex_get_token(struct lexer *l, struct token *tok);

extern void print_token(const struct token *tok);

#endif /* _H */
