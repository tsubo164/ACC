#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static int readc(struct lexer *l)
{
    return fgetc(l->file);
}

static void unreadc(struct lexer *l, int c)
{
    ungetc(c, l->file);
}

static long get_file_pos(struct lexer *l)
{
    return ftell(l->file);
}

static void keyword_or_identifier(struct token *tok)
{
    if (!strcmp(tok->word, "else")) {
        tok->kind = TK_ELSE;
    } else if (!strcmp(tok->word, "if")) {
        tok->kind = TK_IF;
    } else if (!strcmp(tok->word, "int")) {
        tok->kind = TK_INT;
    } else if (!strcmp(tok->word, "return")) {
        tok->kind = TK_RETURN;
    } else if (!strcmp(tok->word, "while")) {
        tok->kind = TK_WHILE;
    } else {
        tok->kind = TK_IDENT;
    }
}

void token_init(struct token *tok)
{
    int i;
    tok->kind = TK_UNKNOWN;
    tok->value = 0;
    tok->file_pos = 0L;
    for (i = 0; i < TOKEN_WORD_SIZE; i++) {
        tok->word[i] = '\0';
    }
}

long token_file_pos(const struct token *tok)
{
    return tok->file_pos;
}

void lexer_init(struct lexer *lex)
{
    lex->file = NULL;
    lex->file_pos = 0L;
}

enum token_kind lex_get_token(struct lexer *l, struct token *tok)
{
    int c = '\0';
    char *wp;
    long tok_pos;

    tok->kind = TK_UNKNOWN;
    tok->value = 0;
    tok->word[0] = '\0';
    wp = tok->word;

state_initial:
    tok_pos = get_file_pos(l);
    c = readc(l);

    switch (c) {

    /* address */
    case '&':
        tok->kind = c;
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
            unreadc(l, c);
            tok->kind = '!';
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
            unreadc(l, c);
            tok->kind = '=';
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
            unreadc(l, c);
            tok->kind = '<';
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
            unreadc(l, c);
            tok->kind = '>';
            break;
        }
        goto state_final;

    /* parenthees */
    case '(': case ')':
        tok->kind = c;
        goto state_final;

    case '{': case '}':
        tok->kind = c;
        goto state_final;

    /* terminator */
    case ';':
        tok->kind = c;
        goto state_final;

    case ',':
        tok->kind = c;
        goto state_final;

    /* whitespaces */
    case ' ': case '\n':
        goto state_initial;

    /* number */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        *wp++ = c;
        goto state_number;

    /* word */
    case '_':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        *wp++ = c;
        goto state_word;

    /* eof */
    case EOF:
        tok->kind = TK_EOF;
        goto state_final;

    /* unknown */
    default:
        tok->kind = TK_UNKNOWN;
        goto state_final;
    }

state_number:
#if 0
    c = readc(l);

    if (isdigit(c)) {
        *wp++ = c;
        goto state_number;
    }
    else {
        *wp = '\0';
        unreadc(l, c);
        tok->kind = TK_NUM;
        tok->value = strtol(tok->word, &wp, 10);
        goto state_final;
    }
#endif
    c = readc(l);

    switch (c) {

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        *wp++ = c;
        goto state_number;

    default:
        *wp = '\0';
        unreadc(l, c);
        tok->kind = TK_NUM;
        tok->value = strtol(tok->word, &wp, 10);
        goto state_final;
    }

state_word:
#if 0
    c = readc(l);

    if (isalnum(c) || c == '_') {
        *wp++ = c;
        goto state_word;
    }
    else {
        *wp = '\0';
        unreadc(l, c);
        tok->kind = TK_IDENT;
        goto state_final;
    }
#endif
    c = readc(l);

    switch (c) {

    case '_':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        *wp++ = c;
        goto state_word;

    default:
        *wp = '\0';
        unreadc(l, c);
        keyword_or_identifier(tok);
        goto state_final;
    }

state_final:
    tok->file_pos = tok_pos;
    return tok->kind;
}
