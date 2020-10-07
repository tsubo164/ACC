#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

enum token_kind {
    TOK_UNKNOWN = -1,
    TOK_END_OF_ASCII = 127,
    TOK_NUM,
    TOK_IDENT,
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

#define TOKEN_WORD_SIZE 128

struct token {
    int kind;
    int value;
    long file_pos;
    char word[TOKEN_WORD_SIZE];
};

struct lexer {
    FILE *file;
    long file_pos;
};

extern void token_init(struct token *tok);
extern long token_file_pos(const struct token *tok);

extern void lexer_init(struct lexer *lex);

extern enum token_kind lex_get_token(struct lexer *l, struct token *tok);

#endif /* _H */
