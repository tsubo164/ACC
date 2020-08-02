#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#if 0
#define TOKEN_LIST(TOK) \
    TOK(TK_IDENT, "identifier") \
    TOK(TK_NUM, "number") \
    TOK(TK_LE, "<=") \
    TOK(TK_GE, ">=") \
    TOK(TK_EQ, "==") \
    TOK(TK_NE, "!=")

enum token_kind {
    TK_UNKNOWN = -1,
    TK_END_OF_ASCII = 127,
#define TOK(tag, str) tag,
    TOKEN_LIST(TOK)
#undef TOK
    TK_EOF
};
#endif

enum token_kind {
    TK_UNKNOWN = -1,
    TK_END_OF_ASCII = 127,
    TK_IDENT,
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
    char word[128];
};

struct lexer {
    FILE *file;
    int buf[2];
};

extern void token_init(struct token *tok);

extern void lexer_init(struct lexer *lex);

extern enum token_kind lex_get_token(struct lexer *l, struct token *tok);

#endif /* _H */
