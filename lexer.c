#include <ctype.h>
#include "lexer.h"

static int readc(struct lexer *l)
{
    l->buf[0] = l->buf[1];
    l->buf[1] = fgetc(l->file);
    return l->buf[1];
}

static int unreadc(struct lexer *l)
{
    ungetc(l->buf[1], l->file);
    l->buf[1] = l->buf[0];
    return l->buf[1];
}

void token_init(struct token *tok)
{
    tok->kind = TK_UNKNOWN;
    tok->value = 0;
}

void lexer_init(struct lexer *lex)
{
    lex->file = NULL;
    lex->buf[0] = '\0';
    lex->buf[1] = '\0';
}

enum token_kind lex_get_token(struct lexer *l, struct token *tok)
{
    int c = '\0';

state_initial:
    c = readc(l);

    switch (c) {

    /* number */
    case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        tok->kind = TK_NUM;
        tok->value = c - '0';
        goto state_final;

    /* arithmetic */
    case '+': case '-': case '*': case '/':
        tok->kind = c;
        goto state_final;

    /* equality */
    case '!':
        c = readc(l);
        switch (c) {
        case '=':
            tok->kind = TK_NE;
            break;
        default:
            tok->kind = unreadc(l);
            break;
        }
        goto state_final;

    case '=':
        c = readc(l);
        switch (c) {
        case '=':
            tok->kind = TK_EQ;
            break;
        default:
            tok->kind = unreadc(l);
            break;
        }
        goto state_final;

    /* relational */
    case '<':
        c = readc(l);
        switch (c) {
        case '=':
            tok->kind = TK_LE;
            break;
        default:
            tok->kind = unreadc(l);
            break;
        }
        goto state_final;

    case '>':
        c = readc(l);
        switch (c) {
        case '=':
            tok->kind = TK_GE;
            break;
        default:
            tok->kind = unreadc(l);
            break;
        }
        goto state_final;

    /* parenthees */
    case '(': case ')':
        tok->kind = c;
        goto state_final;

    /* whitespace */
    case ' ':
        goto state_initial;

    /* eof */
    case EOF:
        tok->kind = TK_EOF;
        goto state_final;

    /* unknown */
    default:
        tok->kind = TK_UNKNOWN;
        goto state_final;
    }

state_final:
    return tok->kind;
}
