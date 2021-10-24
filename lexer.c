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

static int read_escape_sequence(struct lexer *l)
{
    const int c = readc(l);

    switch (c) {
    case '0':  return '\0';
    case '\\': return '\\';
    case '\'': return '\'';
    case 'a':  return '\a';
    case 'b':  return '\b';
    case 'f':  return '\f';
    case 'n':  return '\n';
    case 'r':  return '\r';
    case 't':  return '\t';
    case 'v':  return '\v';
    default:
        /* TODO error/warning */
        return c;
    }
}

static const char *convert_escape_sequence(const char *src, char *dst)
{
    const char *s = src;
    char *d = dst;
    char *end = NULL;

    for (;;) {
        if (*s == '\\') {
            s++;
            switch (*s) {
            case '0':  *d = '\0'; break;
            case '\\': *d = '\\'; break;
            case '\'': *d = '\''; break;
            case 'a':  *d = '\a'; break;
            case 'b':  *d = '\b'; break;
            case 'f':  *d = '\f'; break;
            case 'n':  *d = '\n'; break;
            case 'r':  *d = '\r'; break;
            case 't':  *d = '\t'; break;
            case 'v':  *d = '\v'; break;
            case 'x':
                s++;
                *d = strtol(s, &end, 16);
                s = end - 1;
                break;
            default:
                /* error */
                break;
            }
        } else {
            *d = *s;
        }

        if (!*d)
            break;
        d++;
        s++;
    }

    return dst;
}

static void scan_number(struct lexer *l, struct token *tok)
{
    static char buf[128] = {'\0'};
    char *p = buf;

    for (;;) {
        const int c = readc(l);

        if (isdigit(c)) {
            *p++ = c;
            continue;
        }
        else {
            *p = '\0';
            unreadc(l, c);
            tok->text = make_text(l, buf);
            tok->kind = TOK_NUM;
            tok->value = strtol(tok->text, &p, 10);
            return;
        }
    }
}

static void scan_word(struct lexer *l, struct token *tok)
{
    static char buf[128] = {'\0'};
    char *p = buf;

    for (;;) {
        const int c = readc(l);

        if (isalnum(c) || c == '_') {
            *p++ = c;
            continue;
        }
        else {
            *p = '\0';
            unreadc(l, c);
            tok->text = make_text(l, buf);
            keyword_or_identifier(tok);
            return;
        }
    }
}

static void scan_two_char(struct lexer *l, struct token *tok, char *str, int tok_kind)
{
    const int c = readc(l);

    if (c == str[0]) {
        const int c1 = readc(l);
        if (c1 == str[1]) {
            tok->kind = tok_kind;
        } else {
            unreadc(l, c1);
            tok->kind = c;
        }
        return;
    }
    else {
        unreadc(l, c);
    }
}

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

    /* space */
    if (isspace(c))
        goto state_initial;

    /* number */
    if (isdigit(c)) {
        unreadc(l, c);
        scan_number(l, tok);
        goto state_final;
    }

    /* word */
    if (isalpha(c) || c == '_') {
        unreadc(l, c);
        scan_word(l, tok);
        goto state_final;
    }

    if (c == '=') {
        const int c1 = readc(l);
        if (c1 == '=') {
            tok->kind = TOK_EQ;
        } else {
            unreadc(l, c1);
            tok->kind = c;
        }
        goto state_final;
    }

    if (c == '!') {
        unreadc(l, c);
        scan_two_char(l, tok, "!=", TOK_NE);
        goto state_final;
    }

    if (c == '<') {
        unreadc(l, c);
        scan_two_char(l, tok, "<=", TOK_LE);
        goto state_final;
    }

    if (c == '>') {
        unreadc(l, c);
        scan_two_char(l, tok, ">=", TOK_GE);
        goto state_final;
    }

    if (c == '|') {
        unreadc(l, c);
        scan_two_char(l, tok, "||", TOK_LOGICAL_OR);
        goto state_final;
    }

    if (c == '&') {
        unreadc(l, c);
        scan_two_char(l, tok, "&&", TOK_LOGICAL_AND);
        goto state_final;
    }

    if (strchr("(){}[]:;,?%", c)) {
        tok->kind = c;
        goto state_final;
    }

    /* ================================================= */
    switch (c) {

    /* access */
    case '.':
        {
            const int c2 = readc(l);
            if (c2 == '.') {
                const int c3 = readc(l);
                if (c3 == '.') {
                    tok->kind = TOK_ELLIPSIS;
                    goto state_final;
                } else {
                    unreadc(l, c3);
                }
                unreadc(l, c2);
            } else {
                unreadc(l, c2);
            }
            tok->kind = '.';
            goto state_final;
        }

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

state_string_literal:
    c = readc(l);

    switch (c) {

    case '"':
        {
            static char no_esc_seq[1024] = {'\0'};
            *buf = '\0';
            convert_escape_sequence(textbuf, no_esc_seq);
            tok->text = make_text(l, no_esc_seq);
            tok->kind = TOK_STRING_LITERAL;
        }
        goto state_final;

    case '\\':
        *buf++ = c;
        c = readc(l);
        *buf++ = c;
        goto state_string_literal;

    default:
        *buf++ = c;
        goto state_string_literal;
    }

state_char_literal:
    c = readc(l);

    if (c == '\\')
        /* TODO support hex and octal literal */
        tok->value = read_escape_sequence(l);
    else
        tok->value = c;
    tok->kind = TOK_NUM;

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
        /* ellipsis */
    case TOK_ELLIPSIS: s = "..."; break;
        /* ---- */
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
