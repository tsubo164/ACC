#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "string_table.h"
#include "esc_seq.h"

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
    else if (!strcmp(text, "union"))
        tok->kind = TOK_UNION;
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

void init_token(struct token *tok)
{
    tok->kind = TOK_UNKNOWN;
    tok->value = 0;
    tok->text = NULL;

    init_position(&tok->pos);
}

struct lexer *new_lexer(void)
{
    struct lexer *l = malloc(sizeof(struct lexer));

    l->strtab = new_string_table();
    l->head = NULL;
    l->next = NULL;

    init_position(&l->pos);
    l->prevx = 0;

    return l;
}

void set_source_text(struct lexer *l, const char *text)
{
    l->head = text;
    l->next = text;

    init_position(&l->pos);
    l->prevx = 0;
}

void free_lexer(struct lexer *l)
{
    if (!l)
        return;
    free_string_table(l->strtab);
    free(l);
}

static void read_line_number(struct lexer *l);
static void read_column_number(struct lexer *l);

static int read_escape_sequence(struct lexer *l)
{
    char es[4] = {'\0'};
    int ch = '\0';

    es[0] = '\\';
    es[1] = readc(l);
    es[2] = '\0';

    if (escape_sequence_to_char(es, &ch))
        return ch;
    else
        /* TODO error/warning */
        return es[1];
}

static const char *convert_escape_sequence(const char *src, char *dst)
{
    const char *s = src;
    char *d = dst;
    char *end = NULL;

    for (;;) {
        if (*s == '\\') {
            char es[4] = {'\0'};
            int ch = '\0';

            s++;
            es[0] = '\\';
            es[1] = *s;
            es[2] = '\0';

            if (escape_sequence_to_char(es, &ch)) {
                *d = ch;
            }
            else if (*s == 'x') {
                s++;
                *d = strtol(s, &end, 16);
                s = end - 1;
            }
            else {
                *d = '\\';
                d++;
                *d = *s;
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

static void skip_spaces(struct lexer *l)
{
    for (;;) {
        const int c = readc(l);

        if (!isspace(c)) {
            unreadc(l, c);
            break;
        }
    }
}

static void skip_line_comment(struct lexer *l)
{
    for (;;) {
        const int c = readc(l);

        if (c == '\n')
            break;
    }
}

static void skip_block_comment(struct lexer *l)
{
    for (;;) {
        const int c = readc(l);

        if (c == '*') {
            const int c1 = readc(l);
            if (c1 == '/') {
                return;
            } else {
                unreadc(l, c1);
                continue;
            }
        }
        else if (c == '#') {
            read_column_number(l);
            continue;
        }
        else if (c == EOF) {
            /* TODO error handling */
            printf("error: unterminated /* comment\n");
            return;
        }
    }
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

static void scan_string_literal(struct lexer *l, struct token *tok)
{
    static char buf[1024] = {'\0'};
    char *p = buf;

    for (;;) {
        const int c = readc(l);

        if (c == '"') {
            static char no_esc_seq[1024] = {'\0'};
            *p = '\0';
            convert_escape_sequence(buf, no_esc_seq);
            tok->text = make_text(l, no_esc_seq);
            tok->kind = TOK_STRING_LITERAL;
            return;
        }
        else if (c == '\\') {
            const int c1 = readc(l);
            *p++ = c;
            *p++ = c1;
            continue;
        }
        else {
            *p++ = c;
            continue;
        }
    }
}

static void scan_char_literal(struct lexer *l, struct token *tok)
{
    int c = readc(l);

    if (c == '\\')
        /* TODO support hex and octal literal */
        tok->value = read_escape_sequence(l);
    else
        tok->value = c;
    tok->kind = TOK_NUM;

    c = readc(l);
    if (c != '\'')
        ;/* error */
}

static int match_one_more(struct lexer *l, struct token *tok,
        const char *str, int tok_kind1, int tok_kind2)
{
    int c;

    c = readc(l);
    if (c != str[1]) {
        unreadc(l, c);
        tok->kind = tok_kind1;
        return 0;
    }

    tok->kind = tok_kind2;
    return 1;
}

static int match_two_more(struct lexer *l, struct token *tok,
        const char *str, int tok_kind1, int tok_kind2, int tok_kind3)
{
    int c;

    c = readc(l);
    if (c != str[1]) {
        unreadc(l, c);
        tok->kind = tok_kind1;
        return 0;
    }

    c = readc(l);
    if (c != str[2]) {
        unreadc(l, c);
        tok->kind = tok_kind2;
        return 1;
    }

    tok->kind = tok_kind3;
    return 2;
}

enum token_kind get_next_token(struct lexer *l, struct token *tok)
{
    init_token(tok);

    for (;;) {
        const int c = readc(l);
        tok->pos = l->pos;

        /* space */
        if (isspace(c)) {
            skip_spaces(l);
            continue;
        }

        /* number */
        if (isdigit(c)) {
            unreadc(l, c);
            scan_number(l, tok);
            break;
        }

        /* word */
        if (isalpha(c) || c == '_') {
            unreadc(l, c);
            scan_word(l, tok);
            break;
        }

        /* string literal */
        if (c == '"') {
            scan_string_literal(l, tok);
            break;
        }

        /* char literal */
        if (c == '\'') {
            scan_char_literal(l, tok);
            break;
        }

        /* line number */
        if (c == '#') {
            read_line_number(l);
            continue;
        }

        /* comments */
        if (c == '/') {
            const int c1 = readc(l);
            if (c1 == '/') {
                skip_line_comment(l);
                continue;
            }
            else if (c1 == '*') {
                skip_block_comment(l);
                continue;
            }
            else
                unreadc(l, c1);
            /* no break for arith op */
        }

        /* ellipsis (...) or '.' */
        if (c == '.') {
            const int c2 = readc(l);
            if (c2 == '.') {
                const int c3 = readc(l);
                if (c3 == '.') {
                    tok->kind = TOK_ELLIPSIS;
                    break;
                } else {
                    unreadc(l, c3);
                }
                unreadc(l, c2);
            } else {
                unreadc(l, c2);
            }
            tok->kind = c;
            break;
        }

        if (c == '+') {
            if (match_one_more(l, tok, "++", c, TOK_INC))
                break;
            match_one_more(l, tok, "+=", c, TOK_ADD_ASSIGN);
            break;
        }

        if (c == '-') {
            if (match_one_more(l, tok, "--", c, TOK_DEC))
                break;
            if (match_one_more(l, tok, "-=", c, TOK_SUB_ASSIGN))
                break;
            match_one_more(l, tok, "->", c, TOK_POINTER);
            break;
        }

        if (c == '*') {
            match_one_more(l, tok, "*=", c, TOK_MUL_ASSIGN);
            break;
        }

        if (c == '/') {
            match_one_more(l, tok, "/=", c, TOK_DIV_ASSIGN);
            break;
        }

        if (c == '%') {
            match_one_more(l, tok, "%=", c, TOK_MOD_ASSIGN);
            break;
        }

        if (c == '=') {
            match_one_more(l, tok, "==", c, TOK_EQ);
            break;
        }

        if (c == '!') {
            match_one_more(l, tok, "!=", c, TOK_NE);
            break;
        }

        if (c == '<') {
            if (match_one_more(l, tok, "<=", c, TOK_LE))
                break;
            match_two_more(l, tok, "<<=", c, TOK_SHL, TOK_SHL_ASSIGN);
            break;
        }

        if (c == '>') {
            if (match_one_more(l, tok, ">=", c, TOK_GE))
                break;
            match_two_more(l, tok, ">>=", c, TOK_SHR, TOK_SHR_ASSIGN);
            break;
        }

        if (c == '|') {
            if (match_one_more(l, tok, "||", c, TOK_LOGICAL_OR))
                break;
            match_one_more(l, tok, "|=", c, TOK_OR_ASSIGN);
            break;
        }

        if (c == '&') {
            if (match_one_more(l, tok, "&&", c, TOK_LOGICAL_AND))
                break;
            match_one_more(l, tok, "&=", c, TOK_AND_ASSIGN);
            break;
        }

        if (c == '^') {
            match_one_more(l, tok, "^=", c, TOK_XOR_ASSIGN);
            break;
        }

        if (strchr("(){}[]:;,?~", c)) {
            tok->kind = c;
            break;
        }

        if (c == EOF) {
            tok->kind = TOK_EOF;
            break;
        }

        /* unknown */
        tok->kind = TOK_UNKNOWN;
        tok->value = c;
        break;
    }

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
    case TOK_UNION:
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
    case TOK_TYPE_NAME:
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
    case TOK_SHL: s = "<<"; break;
    case TOK_SHR: s = ">>"; break;
    case TOK_LOGICAL_OR: s = "||"; break;
    case TOK_LOGICAL_AND: s = "&&"; break;
    case TOK_POINTER: s = "->"; break;
        /* assignment op */
    case TOK_ADD_ASSIGN: s = "+="; break;
    case TOK_SUB_ASSIGN: s = "-="; break;
    case TOK_MUL_ASSIGN: s = "*="; break;
    case TOK_DIV_ASSIGN: s = "/="; break;
    case TOK_MOD_ASSIGN: s = "%="; break;
    case TOK_SHL_ASSIGN: s = "<<="; break;
    case TOK_SHR_ASSIGN: s = ">>="; break;
    case TOK_OR_ASSIGN: s = "|="; break;
    case TOK_XOR_ASSIGN: s = "^="; break;
    case TOK_AND_ASSIGN: s = "&="; break;
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
