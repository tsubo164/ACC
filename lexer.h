#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

enum token_kind {
    TK_UNKNOWN = -1,
    TK_END_OF_ASCII = 127,
    TK_NUM,
    TK_LE,
    TK_GE,
    TK_EQ,
    TK_NE,
    TK_EOF
};

struct token {
    int kind;
    int value;
};

struct lexer {
    FILE *file;
    int buf[2];
};

extern void token_init(struct token *tok);

extern void lexer_init(struct lexer *lex);

extern enum token_kind lex_get_token(struct lexer *l, struct token *tok);

#endif /* _H */
