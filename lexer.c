#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static int readc(struct lexer *l)
{
    const int c = *l->next++;

    if (c == '\0')
        return EOF;

    if (c == '\n') {
        l->pos.y++;
        l->prevx = l->pos.x;
        l->pos.x = 0;
    } else {
        l->prevx = l->pos.x;
        l->pos.x++;
    }

    return c;
}

static void unreadc(struct lexer *l, int c)
{
    if (l->pos.x == 0) {
        if (l->pos.y > 1)
            l->pos.y--;
        else
            l->pos.y = 1;
        l->pos.x = l->prevx;
    } else {
        l->pos.x--;
    }

    if (l->next != l->head)
        l->next--;
}

static void keyword_or_identifier(struct token *tok)
{
    const char *text = tok->text;

    if (!strcmp(text, "if"))
        tok->kind = TOK_IF;
    else if (!strcmp(text, "else"))
        tok->kind = TOK_ELSE;
    else if (!strcmp(text, "switch"))
        tok->kind = TOK_SWITCH;
    else if (!strcmp(text, "case"))
        tok->kind = TOK_CASE;
    else if (!strcmp(text, "default"))
        tok->kind = TOK_DEFAULT;

    else if (!strcmp(text, "do"))
        tok->kind = TOK_DO;
    else if (!strcmp(text, "for"))
        tok->kind = TOK_FOR;
    else if (!strcmp(text, "while"))
        tok->kind = TOK_WHILE;

    else if (!strcmp(text, "break"))
        tok->kind = TOK_BREAK;
    else if (!strcmp(text, "continue"))
        tok->kind = TOK_CONTINUE;
    else if (!strcmp(text, "return"))
        tok->kind = TOK_RETURN;
    else if (!strcmp(text, "goto"))
        tok->kind = TOK_GOTO;

    else if (!strcmp(text, "sizeof"))
        tok->kind = TOK_SIZEOF;
    else if (!strcmp(text, "struct"))
        tok->kind = TOK_STRUCT;
    else if (!strcmp(text, "enum"))
        tok->kind = TOK_ENUM;
    else if (!strcmp(text, "typedef"))
        tok->kind = TOK_TYPEDEF;
    else if (!strcmp(text, "extern"))
        tok->kind = TOK_EXTERN;
    else if (!strcmp(text, "static"))
        tok->kind = TOK_STATIC;
    else if (!strcmp(text, "const"))
        tok->kind = TOK_CONST;

    else if (!strcmp(text, "void"))
        tok->kind = TOK_VOID;
    else if (!strcmp(text, "char"))
        tok->kind = TOK_CHAR;
    else if (!strcmp(text, "short"))
        tok->kind = TOK_SHORT;
    else if (!strcmp(text, "int"))
        tok->kind = TOK_INT;
    else if (!strcmp(text, "long"))
        tok->kind = TOK_LONG;
    else if (!strcmp(text, "signed"))
        tok->kind = TOK_SIGNED;
    else if (!strcmp(text, "unsigned"))
        tok->kind = TOK_UNSIGNED;

    else
        tok->kind = TOK_IDENT;
}

static const char *make_text(struct lexer *l, const char *str)
{
    return insert_string(l->strtab, str);
}

static void init_position(struct position *pos)
{
    pos->x = 0;
    pos->y = 1;
    pos->filename = NULL;
}

void token_init(struct token *tok)
{
    tok->kind = TOK_UNKNOWN;
    tok->value = 0;
    tok->text = NULL;

    init_position(&tok->pos);
}

void lexer_init(struct lexer *lex)
{
    lex->strtab = NULL;
    lex->head = NULL;
    lex->next = NULL;

    init_position(&lex->pos);
    lex->prevx = 0;
}

static void read_line_number(struct lexer *l);
static void read_column_number(struct lexer *l);

enum token_kind lex_get_token(struct lexer *l, struct token *tok)
{
    static char textbuf[1024] = {'\0'};
    char *buf;
    int c = '\0';

    token_init(tok);
    textbuf[0] = '\0';
    buf = textbuf;

state_initial:
    c = readc(l);
    tok->pos = l->pos;

    switch (c) {

    /* ---- */
    case '|':
        c = readc(l);
        switch (c) {
        case '|':
            tok->kind = TOK_LOGICAL_OR;
            break;
        default:
            unreadc(l, c);
            tok->kind = '|';
            break;
        }
        goto state_final;

    case '&':
        c = readc(l);
        switch (c) {
        case '&':
            tok->kind = TOK_LOGICAL_AND;
            break;
        default:
            unreadc(l, c);
            tok->kind = '&';
            break;
        }
        goto state_final;

    /* access */
    case '.':
        tok->kind = c;
        goto state_final;

    /* arithmetic */
    case '+':
        c = readc(l);
        switch (c) {
        case '+':
            tok->kind = TOK_INC;
            break;
        case '=':
            tok->kind = TOK_ADD_ASSIGN;
            break;
        default:
            unreadc(l, c);
            tok->kind = '+';
            break;
        }
        goto state_final;

    case '-':
        c = readc(l);
        switch (c) {
        case '-':
            tok->kind = TOK_DEC;
            break;
        case '=':
            tok->kind = TOK_SUB_ASSIGN;
            break;
        case '>':
            tok->kind = TOK_POINTER;
            break;
        default:
            unreadc(l, c);
            tok->kind = '-';
            break;
        }
        goto state_final;

    case '*':
        c = readc(l);
        switch (c) {
        case '=':
            tok->kind = TOK_MUL_ASSIGN;
            break;
        default:
            unreadc(l, c);
            tok->kind = '*';
            break;
        }
        goto state_final;

    case '/':
        c = readc(l);
        switch (c) {
        case '/':
            goto state_line_comment;
        case '*':
            goto state_block_comment;
        case '=':
            tok->kind = TOK_DIV_ASSIGN;
            break;
        default:
            unreadc(l, c);
            tok->kind = '/';
            break;
        }
        goto state_final;

    case '%':
        tok->kind = c;
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

    /* separators */
    case ':': case ';': case ',': case '?':
        tok->kind = c;
        goto state_final;

    /* whitespaces */
    case ' ': case '\n':
        goto state_initial;

    /* number */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        *buf++ = c;
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
        *buf++ = c;
        goto state_word;

    /* string literal */
    case '"':
        goto state_string_literal;

    /* char literal */
    case '\'':
        goto state_char_literal;

    /* eof */
    case EOF:
        tok->kind = TOK_EOF;
        goto state_final;

    case '#':
        read_line_number(l);
        goto state_initial;

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
        *buf++ = c;
        goto state_number;

    default:
        *buf = '\0';
        unreadc(l, c);
        tok->text = make_text(l, textbuf);
        tok->kind = TOK_NUM;
        tok->value = strtol(tok->text, &buf, 10);
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
        *buf++ = c;
        goto state_word;

    default:
        *buf = '\0';
        unreadc(l, c);
        tok->text = make_text(l, textbuf);
        keyword_or_identifier(tok);
        goto state_final;
    }

state_string_literal:
    c = readc(l);

    switch (c) {

    case '"':
        *buf = '\0';
        tok->text = make_text(l, textbuf);
        tok->kind = TOK_STRING_LITERAL;
        goto state_final;

    default:
        *buf++ = c;
        goto state_string_literal;
    }

state_char_literal:
    c = readc(l);

    if (c == '\\') {
        c = readc(l);
        switch (c) {
        case '0':  tok->value = '\0'; break;
        case '\\': tok->value = '\\'; break;
        case '\'': tok->value = '\''; break;
        case 'a':  tok->value = '\a'; break;
        case 'b':  tok->value = '\b'; break;
        case 'f':  tok->value = '\f'; break;
        case 'n':  tok->value = '\n'; break;
        case 'r':  tok->value = '\r'; break;
        case 't':  tok->value = '\t'; break;
        case 'v':  tok->value = '\v'; break;
        default:
            /* error */
            break;
        }
        tok->kind = TOK_NUM;
    } else {
        tok->kind = TOK_NUM;
        tok->value = c;
    }
    c = readc(l);
    if (c != '\'')
        ;/* error */
    goto state_final;

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
        /* TODO error handling */
        unreadc(l, c);
        printf("error: unterminated /* comment\n");
        goto state_initial;

    case '#':
        read_column_number(l);
        goto state_block_comment;

    default:
        goto state_block_comment;
    }

state_final:
    return tok->kind;
}

void print_token(const struct token *tok)
{
    const char *s;

    printf("(%d, %d) => ", tok->pos.y, tok->pos.x);

    if (tok->kind == '\n') {
        printf("\"\\n\"\n");
        return;
    }

    if (tok->kind < TOK_END_OF_ASCII) {
        printf("\"%c\"\n", tok->kind);
        return;
    }

    switch (tok->kind) {
    case TOK_NUM:
    case TOK_IDENT:
    case TOK_STRING_LITERAL:
         /* keywords */
    case TOK_IF:
    case TOK_ELSE:
    case TOK_SWITCH:
    case TOK_CASE:
    case TOK_DEFAULT:
    case TOK_DO:
    case TOK_FOR:
    case TOK_WHILE:
    case TOK_BREAK:
    case TOK_CONTINUE:
    case TOK_RETURN:
    case TOK_GOTO:
    case TOK_SIZEOF:
    case TOK_STRUCT:
    case TOK_ENUM:
    case TOK_TYPEDEF:
    case TOK_EXTERN:
    case TOK_STATIC:
    case TOK_CONST:
         /* types */
    case TOK_VOID:
    case TOK_CHAR:
    case TOK_SHORT:
    case TOK_INT:
    case TOK_LONG:
    case TOK_SIGNED:
    case TOK_UNSIGNED:
        printf("\"%s\"\n", tok->text);
        return;

    default:
        break;
    }

    switch (tok->kind) {

         /* unary op */
    case TOK_INC: s = "++"; break;
    case TOK_DEC: s = "--"; break;
         /* bin op */
    case TOK_LE: s = "<="; break;
    case TOK_GE: s = ">="; break;
    case TOK_EQ: s = "=="; break;
    case TOK_NE: s = "!="; break;
    case TOK_LOGICAL_OR: s = "||"; break;
    case TOK_LOGICAL_AND: s = "&&"; break;
    case TOK_POINTER: s = "->"; break;
         /* assignment op */
    case TOK_ADD_ASSIGN: s = "+="; break;
    case TOK_SUB_ASSIGN: s = "+="; break;
    case TOK_MUL_ASSIGN: s = "+="; break;
    case TOK_DIV_ASSIGN: s = "+="; break;
    case TOK_EOF: s = "EOF"; break;

    default:
        break;
    }
    printf("\"%s\"\n", s);
}

static void read_spaces(struct lexer *l)
{
    for (;;) {
        const int c = readc(l);
        if (c == ' '  ||
            c == '\t' ||
            c == '\v' ||
            c == '\f') {
            continue;
        } else {
            unreadc(l, c);
            break;
        }
    }
}

static void read_char(struct lexer *l, int expected)
{
    const int c = readc(l);
    if (c != expected) {
        /* error */
    }
}

static int read_number(struct lexer *l)
{
    int num = 0;

    for (;;) {
        const int c = readc(l);
        if (isdigit(c)) {
            num = num * 10 + c - '0';
        } else {
            unreadc(l, c);
            break;
        }
    }

    return num;
}

static void read_file_path(struct lexer *l, char *path)
{
    char *buf = path;

    for (;;) {
        const int c = readc(l);
        if (isalnum(c) || c == '_' || c == '-' || c == '.' || c == '/') {
            *buf++ = c;
        } else {
            *buf = '\0';
            unreadc(l, c);
            break;
        }
    }
}

static void read_line_number(struct lexer *l)
{
    static char path[256] = {'\0'};
    int num = 0;

    read_spaces(l);

    num = read_number(l);

    read_spaces(l);

    read_char(l, '"');
    read_file_path(l, path);
    read_char(l, '"');

    read_spaces(l);

    read_char(l, '\n');

    l->pos.y = num;
    l->pos.filename = make_text(l, path);
}

static void read_column_number(struct lexer *l)
{
    const int col = read_number(l);
    l->pos.x = col;
}
