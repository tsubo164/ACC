#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "preprocessor.h"

#define TERMINAL_COLOR_BLACK   "\x1b[30m"
#define TERMINAL_COLOR_RED     "\x1b[31m"
#define TERMINAL_COLOR_GREEN   "\x1b[32m"
#define TERMINAL_COLOR_YELLOW  "\x1b[33m"
#define TERMINAL_COLOR_BLUE    "\x1b[34m"
#define TERMINAL_COLOR_MAGENTA "\x1b[35m"
#define TERMINAL_COLOR_CYAN    "\x1b[36m"
#define TERMINAL_COLOR_WHITE   "\x1b[37m"
#define TERMINAL_COLOR_RESET   "\x1b[39m"

#define TERMINAL_DECORATION_BOLD    "\x1b[1m"
#define TERMINAL_DECORATION_RESET   "\x1b[0m"

void strbuf_init(struct strbuf *s);
void strbuf_copy(struct strbuf *s, const char *c);
void strbuf_append(struct strbuf *s, const char *c);
void strbuf_grow(struct strbuf *s, size_t new_len);
void strbuf_free(struct strbuf *s);

void strbuf_init(struct strbuf *s)
{
    s->buf = NULL;
    s->len = 0;
    s->alloc = 0;
}

void strbuf_copy(struct strbuf *s, const char *c)
{
    const size_t new_len = strlen(c);

    strbuf_grow(s, new_len);
    strcpy(s->buf, c);
    s->len = new_len;
}

void strbuf_append(struct strbuf *s, const char *c)
{
    const char *pp;
    char *dst;
    const size_t new_len = s->len + strlen(c);

    strbuf_grow(s, new_len);

    pp = c;
    dst = &s->buf[s->len];
    while (*pp)
        *dst++ = *pp++;
    *dst = '\0';

    s->len = new_len;
}

void strbuf_append_char(struct strbuf *s, char c)
{
    const size_t new_len = s->len + 1;
    strbuf_grow(s, new_len);
    s->buf[s->len]  = c;
    s->buf[new_len] = '\0';
    s->len = new_len;
}

void strbuf_grow(struct strbuf *s, size_t new_len)
{
    static const size_t INIT_BUF_SIZE = 1024 * 16;

    if (s->alloc < new_len + 1) {
        const size_t new_alloc = s->alloc == 0 ? INIT_BUF_SIZE : s->alloc * 2;
        s->buf = (char *) realloc(s->buf, sizeof(char) * new_alloc);
        s->alloc = new_alloc;
    }
}

void strbuf_free(struct strbuf *s)
{
    if (s->alloc > 0) {
        free(s->buf);
        strbuf_init(s);
    }
}

static unsigned int hash_fn(const char *key)
{
    unsigned int h = 0;
    unsigned const char *p = NULL;

    for (p = (unsigned const char *) key; *p != '\0'; p++)
        h = PP_MULTIPLIER * h + *p;

    return h % PP_HASH_SIZE;
}

static struct macro_param *new_param(const char *name)
{
    struct macro_param *param = malloc(sizeof(struct macro_param));
    const size_t alloc = strlen(name) + 1;
    char *dst = malloc(sizeof(char) * alloc);

    strncpy(dst, name, alloc);
    param->name = dst;
    param->arg[0] = '\0';
    param->next = NULL;

    return param;
}

static void free_param(struct macro_param *param)
{
    if (!param)
        return;

    free_param(param->next);
    free(param->name);
    free(param);
}

static struct macro_entry *new_entry(const char *name)
{
    struct macro_entry *entry = malloc(sizeof(struct macro_entry));
    const size_t alloc = strlen(name) + 1;
    char *dst = malloc(sizeof(char) * alloc);

    strncpy(dst, name, alloc);
    entry->name = dst;
    entry->repl = NULL;
    entry->is_func = 0;
    entry->params = NULL;
    entry->next = NULL;

    return entry;
}

static void free_entry(struct macro_entry *entry)
{
    if (!entry)
        return;

    free_entry(entry->next);
    free(entry->name);
    free(entry->repl);
    free_param(entry->params);
    free(entry);
}

static struct macro_table *new_macro_table(void)
{
    struct macro_table *table = malloc(sizeof(struct macro_table));
    int i;

    for (i = 0; i < PP_HASH_SIZE; i++)
        table->entries[i] = NULL;

    return table;
}

static void free_macro_table(struct macro_table *table)
{
    int i;

    if (!table)
        return;

    for (i = 0; i < PP_HASH_SIZE; i++)
        free_entry(table->entries[i]);

    free(table);
}

static struct macro_entry *lookup_macro(struct macro_table *table, const char *name)
{
    struct macro_entry *entry = NULL;
    const unsigned int h = hash_fn(name);

    for (entry = table->entries[h]; entry != NULL; entry = entry->next) {
        if (!strcmp(name, entry->name))
            return entry;
    }
    return NULL;
}

static struct macro_entry *insert_macro(struct macro_table *table, const char *name)
{
    struct macro_entry *entry = NULL;
    const unsigned int h = hash_fn(name);

    for (entry = table->entries[h]; entry != NULL; entry = entry->next) {
        if (!strcmp(name, entry->name))
            return entry;
    }

    entry = new_entry(name);

    entry->next = table->entries[h];
    table->entries[h] = entry;

    return entry;
}

static void add_replacement(struct macro_entry *mac, const char *repl)
{
    const size_t alloc = strlen(repl) + 1;
    char *dst = malloc(sizeof(char) * alloc);

    free(mac->repl);

    strncpy(dst, repl, alloc);
    mac->repl = dst;
}

struct preprocessor *new_preprocessor(void)
{
    struct preprocessor *pp;

    pp = malloc(sizeof(struct preprocessor));

    pp->text = malloc(sizeof(struct strbuf));
    strbuf_init(pp->text);
    pp->mactab = new_macro_table(); 
    pp->filename = NULL;
    pp->y = 1;
    pp->x = 0;
    pp->prevx = 0;

    pp->skip_depth = 0;

    return pp;
}

void free_preprocessor(struct preprocessor *pp)
{
    if (!pp)
        return;

    strbuf_free(pp->text);
    free(pp->text);
    free_macro_table(pp->mactab);
    free(pp);
}

static void error_(struct preprocessor *pp, const char *msg)
{
    fprintf(stderr, TERMINAL_DECORATION_BOLD);
        fprintf(stderr, "%s:", pp->filename);
        fprintf(stderr, "%d:%d: ", pp->y, pp->x);
        fprintf(stderr, TERMINAL_COLOR_RED);
            fprintf(stderr, "%s: ", "error");
        fprintf(stderr, TERMINAL_COLOR_RESET);
        fprintf(stderr, "%s\n", msg);
    fprintf(stderr, TERMINAL_DECORATION_RESET);
}

/* forward declarations */
static void text_lines(struct preprocessor *pp);
static void endif_line(struct preprocessor *pp);
static void whitespaces(struct preprocessor *pp);

static int is_skipping(struct preprocessor *pp)
{
    return pp->skip_depth > 0;
}

static void writec(struct preprocessor *pp, char c)
{
    if (!is_skipping(pp) || c == '\n')
        strbuf_append_char(pp->text, c);
}

static void writes(struct preprocessor *pp, const char *c)
{
    if (!is_skipping(pp)) {
        const char *p;
        for (p = c; *p; p++)
            strbuf_append_char(pp->text, *p);
    }
}

static void write_line_directive(struct preprocessor *pp)
{
    static char buf[256] = {'\0'};

    sprintf(buf, "# %d \"%s\"\n", pp->y, pp->filename);
    writes(pp, buf);
}

static void write_column_comment(struct preprocessor *pp)
{
    char buf[32] = {'\0'};
    const int pos = pp->x - 2; /* minus closing 'star and slash' */
    sprintf(buf, "/*#%d*/", pos);
    writes(pp, buf);
}

static int getc_(struct preprocessor *pp)
{
    const int c = fgetc(pp->fp);
    pp->x++;

    if (c == '\n') {
        pp->y++;
        pp->prevx = pp->x;
        pp->x = 0;
    }

    return c;
}

static void ungetc_(struct preprocessor *pp, int c)
{
    ungetc(c, pp->fp);
    if (c == '\n') {
        pp->y--;
        pp->x = pp->prevx;
    }
    else {
        pp->x--;
    }
}

static int readc(struct preprocessor *pp)
{
    static int escapednl = 0;
    const int c = getc_(pp);

    if (c == '\n') {
        if (escapednl > 0) {
            /* pop nl from stack and put the nl back to stream
             * that was just read. use ungetc as no need to decrease x */
            escapednl--;
            ungetc(c, pp->fp);
        }
    }
    else if (c == '\\') {
        const int c1 = getc_(pp);

        if (c1 == '\n') {
            /* push nl to stack and read one more char */
            escapednl++;
            return readc(pp);
        } else {
            ungetc_(pp, c1);
        }
    }

    return c;
}

static void unreadc(struct preprocessor *pp, int c)
{
    ungetc_(pp, c);
}

static int is_whitespaces(int c)
{
    /* whitespaces other than new line */
    return c == ' ' || c == '\t' || c == '\v' || c == '\f';
}

static int is_macroname(int c)
{
    return isalnum(c) || c == '_' || c == '$';
}

static void new_line(struct preprocessor *pp)
{
    int c;
    whitespaces(pp);

    c = readc(pp);
    if (c != '\n')
        error_(pp, "expected a new line");

    writec(pp, c);
}

static void line_comment(struct preprocessor *pp)
{
    for (;;) {
        const int c = readc(pp);

        if (c == '\n') {
            unreadc(pp, c);
            break;
        }
    }
}

static void block_comment(struct preprocessor *pp)
{
    for (;;) {
        const int c = readc(pp);

        if (c == '*') {
            const int c1 = readc(pp);

            if (c1 == '/')
                break;
        }
        else if (c == '\n') {
            writec(pp, c);
        }
        else if (c == EOF) {
            error_(pp, "unterminated /* comment");
            break;
        }
    }
}

static void string_literal(struct preprocessor *pp)
{
    for (;;) {
        const int c = readc(pp);

        if (c == '"') {
            writec(pp, c);
            break;
        }
        else if (c == EOF) {
            error_(pp, "unterminated \" for string literal");
            break;
        }
        else if (c == '\\') {
            const int c1 = readc(pp);
            writec(pp, c);
            writec(pp, c1);
        }
        else {
            writec(pp, c);
        }
    }
}

static void character_literal(struct preprocessor *pp)
{
    for (;;) {
        const int c = readc(pp);

        if (c == '\'') {
            writec(pp, c);
            break;
        }
        else if (c == EOF) {
            error_(pp, "unterminated ' for character literal");
            break;
        }
        else if (c == '\\') {
            const int c1 = readc(pp);
            writec(pp, c);
            writec(pp, c1);
        }
        else {
            writec(pp, c);
        }
    }
}

static void whitespaces(struct preprocessor *pp)
{
    for (;;) {
        const int c = readc(pp);

        if (c == '/') {
            const int c1 = readc(pp);
            if (c1 == '*') {
                block_comment(pp);
                continue;
            }
            else if (c1 == '/') {
                line_comment(pp);
                continue;
            }
            else {
                unreadc(pp, c1);
            }
            unreadc(pp, c);
            break;
        }

        if (!is_whitespaces(c)) {
            unreadc(pp, c);
            break;
        }
    }
}

static void token(struct preprocessor *pp, char *buf)
{
    char *p = buf;

    whitespaces(pp);

    for (;;) {
        const int c = readc(pp);
        if (is_macroname(c)) {
            *p++ = c;
        } else {
            unreadc(pp, c);
            break;
        }
    }
    *p = '\0';
}

static void token_list(struct preprocessor *pp, char *buf)
{
    char *p = buf;

    whitespaces(pp);

    for (;;) {
        const int c = readc(pp);

        if (c == '\n') {
            unreadc(pp, c);
            break;
        }

        *p++ = c;
    }
    *p = '\0';
}

static void read_file_path(struct preprocessor *pp, char *path)
{
    char *buf = path;

    for (;;) {
        const int c = readc(pp);
        if (isalnum(c) || c == '_' || c == '-' || c == '.' || c == '/') {
            *buf++ = c;
        } else {
            *buf = '\0';
            unreadc(pp, c);
            break;
        }
    }
}

static void file_path(struct preprocessor *pp, char *buf)
{
    int c, quot;

    whitespaces(pp);

    c = readc(pp);
    if (c == '"')
        quot = '"';
    else if (c == '<')
        quot = '>';
    else
        goto file_path_error;

    read_file_path(pp, buf);

    c = readc(pp);
    if (c != quot)
        goto file_path_error;

    if (quot == '>') {
        static char tmp[128] = {'\0'};
        strcpy(tmp, buf);
        sprintf(buf, "include/%s", tmp);
    }

    return;

file_path_error:
    error_(pp, "expected \"FILENAME\" or <FILENAME>");
    return;
}

static void const_expression(struct preprocessor *pp)
{
    static char expr[128] = {'\0'};
    token(pp, expr);
}

static void include_line(struct preprocessor *pp)
{
    static char filename[256] = {'\0'};

    file_path(pp, filename);

    new_line(pp);

    preprocess_text(pp, filename);
    write_line_directive(pp);
}

static void if_part(struct preprocessor *pp)
{
    const_expression(pp);
    new_line(pp);
}

static void ifndef_part(struct preprocessor *pp)
{
    static char ident[128] = {'\0'};

    token(pp, ident);
    new_line(pp);

    if (lookup_macro(pp->mactab, ident))
        pp->skip_depth++;
}

static void endif_line(struct preprocessor *pp)
{
    new_line(pp);

    if (pp->skip_depth)
        pp->skip_depth--;
}

static struct macro_param *parameter_list(struct preprocessor *pp)
{
    int c = readc(pp);

    if (c == '(') {
        struct macro_param head = {0};
        struct macro_param *tail = &head;
        char name[128] = {'\0'};

        for (;;) {
            struct macro_param *param;

            token(pp, name);

            param = new_param(name);
            tail->next = param;
            tail = param;

            whitespaces(pp);

            c = readc(pp);
            if (c == ')')
                break;
            else if (c == ',')
                continue;
        }
        return head.next;

    } else {
        unreadc(pp, c);
        return NULL;
    }
}

static void define_line(struct preprocessor *pp)
{
    static char ident[128] = {'\0'};
    static char repl[1024] = {'\0'};
    struct macro_entry *mac = NULL;
    struct macro_param *params = NULL;

    token(pp, ident);

    params = parameter_list(pp);

    token_list(pp, repl);
    new_line(pp);

    mac = lookup_macro(pp->mactab, ident);
    if (mac) {
        if (strcmp(mac->repl, repl)) {
            /* override */
            add_replacement(mac, repl);
            /* TODO generate warning */
            error_(pp, "'' macro redefined\n");
        }
    } else {
        mac = insert_macro(pp->mactab, ident);
        add_replacement(mac, repl);

        mac->is_func = (params != NULL);
        mac->params = params;
    }
}

static void unknown_directive(struct preprocessor *pp, const char *direc)
{
    writes(pp, "# ");
    writes(pp, direc);

    for (;;) {
        const int c = readc(pp);
        writec(pp, c);
        if (c == '\n')
            break;
    }
}

static void directive_line(struct preprocessor *pp)
{
    static char direc[128] = {'\0'};

    token(pp, direc);

    if (!strcmp(direc, "include") && !is_skipping(pp))
        include_line(pp);
    else if (!strcmp(direc, "define") && !is_skipping(pp))
        define_line(pp);
    else if (!strcmp(direc, "if"))
        if_part(pp);
    else if (!strcmp(direc, "ifndef"))
        ifndef_part(pp);
    else if (!strcmp(direc, "endif"))
        endif_line(pp);
    else
        unknown_directive(pp, direc);
}

static void argument(struct preprocessor *pp, char *buf)
{
    char *p = buf;

    whitespaces(pp);

    for (;;) {
        const int c = readc(pp);
        /* TODO push/pop '(', ')' */
        if (!is_whitespaces(c) && c != ')' && c != ',') {
            *p++ = c;
        } else {
            unreadc(pp, c);
            break;
        }
    }
    *p = '\0';
}

static void read_arguments(struct preprocessor *pp, struct macro_param *params)
{
    struct macro_param *p = params;
    char arg[128] = {'\0'};
    int c;

    whitespaces(pp);

    c = readc(pp);
    if (c != '(') {
        /* TODO error */
        return;
    }

    for (;;) {
        if (!p) {
            /* TODO error */
            return;
        }

        argument(pp, arg);
        strcpy(p->arg, arg);
        p = p->next;

        whitespaces(pp);

        c = readc(pp);
        if (c == ')')
            return;
        else if (c == ',')
            continue;
    }
}

static struct macro_param *find_param(struct macro_param *params, const char *name)
{
    struct macro_param *p;
    for (p = params; p; p = p->next)
        if (!strcmp(p->name, name))
            return p;
    return NULL;
}

static void expand_function(struct preprocessor *pp, struct macro_entry *mac)
{
    char token[32] = {'\0'};
    char *t = token;
    char *ch;

    for (ch = mac->repl; *ch; ch++) {
        if (isalnum(*ch) || *ch == '_') {
            *t = *ch;
            t++;
        } else {
            *t = '\0';
            if (t != token) {
                struct macro_param *prm = find_param(mac->params, token);
                if (prm)
                    writes(pp, prm->arg);
                else
                    writes(pp, token);
            }
            t = token;
            writec(pp, *ch);
        }
    }
}

static void expand(struct preprocessor *pp)
{
    static char tok[128] = {'\0'};
    struct macro_entry *mac;

    token(pp, tok);

    mac = lookup_macro(pp->mactab, tok);

    if (mac && mac->repl) {
        if (mac->is_func) {
            read_arguments(pp, mac->params);
            expand_function(pp, mac);
        } else {
            writes(pp, mac->repl);
        }
    } else {
        writes(pp, tok);
    }
}

static void text_lines(struct preprocessor *pp)
{
    for (;;) {
        const int c = readc(pp);

        if (c == '/') {
            const int c1 = readc(pp);
            if (c1 == '/') {
                line_comment(pp);
                continue;
            }
            else if (c1 == '*') {
                const int starty = pp->y;
                block_comment(pp);
                if (pp->y == starty)
                    write_column_comment(pp);
                continue;
            }
            else {
                unreadc(pp, c1);
                writec(pp, c);
                continue;
            }
        }
        else if (c == '"') {
            writec(pp, c);
            string_literal(pp);
            continue;
        }
        else if (c == '\'') {
            writec(pp, c);
            character_literal(pp);
            continue;
        }
        else if (c == '#') {
            directive_line(pp);
            continue;
        }
        else if (is_macroname(c)) {
            unreadc(pp, c);
            expand(pp);
            continue;
        }
        else if (c == EOF) {
            break;
        }
        else {
            writec(pp, c);
        }
    }
}

int preprocess_text(struct preprocessor *pp, const char *filename)
{
    FILE *fp;

    fp = fopen(filename, "r");
    if (!fp)
        return 1;

    {
        struct preprocessor new_pp = *pp;
        new_pp.y = 1;
        new_pp.x = 0;
        new_pp.filename = filename;
        new_pp.fp = fp;

        write_line_directive(&new_pp);
        text_lines(&new_pp);
    }

    fclose(fp);

    return 0;
}
