#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "string_table.h"
#include "position.h"

enum token_kind {
    TOK_UNKNOWN = -1,
    TOK_END_OF_ASCII = 127,
    TOK_NUM,
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
    /* unary op */
    TOK_INC,
    TOK_DEC,
    /* bin op */
    TOK_LE,
    TOK_GE,
    TOK_EQ,
    TOK_NE,
    TOK_LOGICAL_OR,
    TOK_LOGICAL_AND,
    TOK_POINTER,
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
    int value;
    const char *text;
    struct position pos;
};

struct lexer {
    struct string_table *strtab;
    const char *head;
    const char *next;

    struct position pos;
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
