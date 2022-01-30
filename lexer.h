#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "position.h"

enum token_kind {
    TOK_UNKNOWN = -1,
    TOK_END_OF_ASCII = 127,
    TOK_NUM,
    TOK_FPNUM,
    TOK_IDENT,
    TOK_STRING_LITERAL,
    /* keywords */
    TOK_IF,
    TOK_ELSE,
    TOK_SWITCH,
    TOK_CASE,
    TOK_DEFAULT,
    TOK_DO,
    TOK_FOR,
    TOK_WHILE,
    TOK_BREAK,
    TOK_CONTINUE,
    TOK_RETURN,
    TOK_GOTO,
    TOK_SIZEOF,
    TOK_STRUCT,
    TOK_UNION,
    TOK_ENUM,
    TOK_TYPEDEF,
    TOK_EXTERN,
    TOK_STATIC,
    TOK_CONST,
    /* types */
    TOK_VOID,
    TOK_CHAR,
    TOK_SHORT,
    TOK_INT,
    TOK_LONG,
    TOK_FLOAT,
    TOK_DOUBLE,
    TOK_SIGNED,
    TOK_UNSIGNED,
    TOK_TYPE_NAME,
    /* unary op */
    TOK_INC,
    TOK_DEC,
    /* bin op */
    TOK_LE,
    TOK_GE,
    TOK_EQ,
    TOK_NE,
    TOK_SHL,
    TOK_SHR,
    TOK_LOGICAL_OR,
    TOK_LOGICAL_AND,
    TOK_POINTER,
    /* assignment op */
    TOK_ADD_ASSIGN,
    TOK_SUB_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    TOK_MOD_ASSIGN,
    TOK_SHL_ASSIGN,
    TOK_SHR_ASSIGN,
    TOK_OR_ASSIGN,
    TOK_XOR_ASSIGN,
    TOK_AND_ASSIGN,
    /* ellipsis */
    TOK_ELLIPSIS,
    /* ---- */
    TOK_EOF
};

struct token {
    int kind;
    long value;
    float fpnum;
    const char *text;
    struct position pos;
};

struct string_table;

struct lexer {
    struct string_table *strtab;
    const char *head;
    const char *next;

    struct position pos;
    int prevx;
};

extern void init_token(struct token *tok);
extern void print_token(const struct token *tok);

extern struct lexer *new_lexer(void);
extern void free_lexer(struct lexer *l);

extern enum token_kind get_next_token(struct lexer *l, struct token *tok);
extern void set_source_text(struct lexer *l, const char *text);

#endif /* _H */
