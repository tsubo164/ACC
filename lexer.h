#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

enum token_kind {
    TK_UNKNOWN = -1,
    TK_END_OF_ASCII = 127,
    TK_NUM,
    TK_IDENT,
    /* keyword */
    TK_ELSE,
    TK_IF,
    TK_RETURN,
    TK_WHILE,
    /* bin op */
    TK_LE,
    TK_GE,
    TK_EQ,
    TK_NE,
    TK_EOF
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
