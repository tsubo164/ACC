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
    const char *text = tok->text;

    if (!strcmp(text, "char")) {
        tok->kind = TOK_CHAR;
    } else if (!strcmp(text, "else")) {
        tok->kind = TOK_ELSE;
    } else if (!strcmp(text, "if")) {
        tok->kind = TOK_IF;
    } else if (!strcmp(text, "int")) {
        tok->kind = TOK_INT;
    } else if (!strcmp(text, "return")) {
        tok->kind = TOK_RETURN;
    } else if (!strcmp(text, "struct")) {
        tok->kind = TOK_STRUCT;
    } else if (!strcmp(text, "while")) {
        tok->kind = TOK_WHILE;
    } else {
        tok->kind = TOK_IDENT;
    }
}

static const char *make_text(struct lexer *l, const char *str)
{
    return insert_string(l->strtab, str);
}

void token_init(struct token *tok)
{
    tok->kind = TOK_UNKNOWN;
    tok->value = 0;
    tok->file_pos = 0L;
    tok->text = NULL;
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
    static char textbuf[1024] = {'\0'};
    char *tp;
    int c = '\0';
    long tok_pos;

    token_init(tok);

    textbuf[0] = '\0';
    tp = textbuf;

state_initial:
    c = readc(l);
    tok_pos = get_file_pos(l);

    switch (c) {

    /* address */
    case '&':
        tok->kind = c;
        goto state_final;

    /* access */
    case '.':
        tok->kind = c;
        goto state_final;

    /* arithmetic */
    case '+': case '-': case '*':
        tok->kind = c;
        goto state_final;

    case '/':
        c = readc(l);
        switch (c) {
        case '/':
            goto state_line_comment;
        case '*':
            goto state_block_comment;
        default:
            unreadc(l, c);
            tok->kind = '/';
            break;
        }
        goto state_final;

    /* equality */
    case '!':
        c = readc(l);
        switch (c) {
        case '=':
            tok->kind = TOK_NE;
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
            tok->kind = TOK_EQ;
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
            tok->kind = TOK_LE;
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
            tok->kind = TOK_GE;
            break;
        default:
            unreadc(l, c);
            tok->kind = '>';
            break;
        }
        goto state_final;

    /* parenthees */
    case '(': case ')':
    case '{': case '}':
    case '[': case ']':
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
        *tp++ = c;
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
        *tp++ = c;
        goto state_word;

    /* eof */
    case EOF:
        tok->kind = TOK_EOF;
        goto state_final;

    /* unknown */
    default:
        tok->kind = TOK_UNKNOWN;
        goto state_final;
    }

state_number:
    c = readc(l);

    switch (c) {

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        *tp++ = c;
        goto state_number;

    default:
        *tp = '\0';
        unreadc(l, c);
        tok->text = make_text(l, textbuf);
        tok->kind = TOK_NUM;
        tok->value = strtol(tok->text, &tp, 10);
        goto state_final;
    }

state_word:
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
        *tp++ = c;
        goto state_word;

    default:
        *tp = '\0';
        unreadc(l, c);
        tok->text = make_text(l, textbuf);
        keyword_or_identifier(tok);
        goto state_final;
    }

state_line_comment:
    c = readc(l);

    switch (c) {

    case '\n':
        goto state_initial;

    default:
        goto state_line_comment;
    }

state_block_comment:
    c = readc(l);

    switch (c) {

    case '*':
        c = readc(l);
        switch (c) {
        case '/':
            goto state_initial;
        default:
            unreadc(l, c);
            goto state_block_comment;
        }

    case EOF:
        /* XXX tmp */
        unreadc(l, c);
        printf("error: unterminated /* comment\n");
        goto state_initial;

    default:
        goto state_block_comment;
    }

state_final:
    tok->file_pos = tok_pos;
    return tok->kind;
}
