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

void strbuf_init(struct strbuf *s, size_t reserve_len);
void strbuf_copy(struct strbuf *s, const char *c);
void strbuf_append(struct strbuf *s, const char *c);
void strbuf_grow(struct strbuf *s, size_t new_len);
void strbuf_free(struct strbuf *s);

static void zero_clear(struct strbuf *s)
{
    s->buf = NULL;
    s->len = 0;
    s->alloc = 0;
}

void strbuf_init(struct strbuf *s, size_t reserve_len)
{
    zero_clear(s);
    strbuf_grow(s, reserve_len);
    s->buf[0] = '\0';
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

    if (c == '\0')
        return;

    strbuf_grow(s, new_len);
    s->buf[s->len]  = c;
    s->buf[new_len] = '\0';
    s->len = new_len;
}

void strbuf_grow(struct strbuf *s, size_t new_len)
{
#define INIT_BUF_SIZE 32
    size_t new_alloc = s->alloc == 0 ? INIT_BUF_SIZE : s->alloc;

    while (new_alloc < new_len + 1)
        new_alloc *= 2;

    if (new_alloc == s->alloc)
        return;

    s->buf = (char *) realloc(s->buf, sizeof(char) * new_alloc);
    s->alloc = new_alloc;
}

void strbuf_free(struct strbuf *s)
{
    if (s->alloc > 0) {
        free(s->buf);
        zero_clear(s);
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
    strbuf_init(&param->arg, 0);
    param->next = NULL;

    return param;
}

static void free_param(struct macro_param *param)
{
    if (!param)
        return;

    free_param(param->next);
    free(param->name);
    strbuf_free(&param->arg);
    free(param);
}

static struct macro_entry *new_entry(const char *name)
{
    struct macro_entry *ent = malloc(sizeof(struct macro_entry));
    const size_t alloc = strlen(name) + 1;
    char *dst = malloc(sizeof(char) * alloc);

    strncpy(dst, name, alloc);
    ent->name = dst;
    ent->repl = NULL;
    ent->is_func = 0;
    ent->params = NULL;
    ent->next = NULL;

    return ent;
}

static void free_entry(struct macro_entry *entry)
{
    if (!entry)
        return;

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

    for (i = 0; i < PP_HASH_SIZE; i++) {
        struct macro_entry *ent = table->entries[i], *tmp;
        if (!ent)
            continue;

        while (ent) {
            tmp = ent->next;
            free_entry(ent);
            ent = tmp;
        }
    }

    free(table);
}

static struct macro_entry *lookup_macro(struct macro_table *table, const char *name)
{
    struct macro_entry *ent = NULL;
    const unsigned int h = hash_fn(name);

    for (ent = table->entries[h]; ent; ent = ent->next)
        if (!strcmp(name, ent->name))
            return ent;

    return NULL;
}

static struct macro_entry *insert_macro(struct macro_table *table, const char *name)
{
    struct macro_entry *ent = NULL;
    const unsigned int h = hash_fn(name);

    for (ent = table->entries[h]; ent; ent = ent->next)
        if (!strcmp(name, ent->name))
            return ent;

    ent = new_entry(name);
    ent->next = table->entries[h];
    table->entries[h] = ent;

    return ent;
}

static void add_replacement(struct macro_entry *mac, const char *repl)
{
    const size_t alloc = strlen(repl) + 1;
    char *dst = malloc(sizeof(char) * alloc);

    free(mac->repl);

    strncpy(dst, repl, alloc);
    mac->repl = dst;
}

static void reset_hideset(struct preprocessor *pp)
{
    int i;
    for (i = 0; i < MAX_HIDESET; i++)
        pp->hideset[i] = NULL;
}

struct preprocessor *new_preprocessor(void)
{
    struct preprocessor *pp;

    pp = malloc(sizeof(struct preprocessor));

    pp->text = malloc(sizeof(struct strbuf));
    strbuf_init(pp->text, 1024 * 16 - 1);
    pp->mactab = new_macro_table(); 
    pp->filename = NULL;
    pp->y = 1;
    pp->x = 0;
    pp->prevx = 0;

    pp->skip_depth = 0;

    reset_hideset(pp);
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

const char *get_text(const struct preprocessor *pp)
{
    if (!pp || !pp->text)
        return NULL;
    return pp->text->buf;
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
            else
                unreadc(pp, c1);
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

    preprocess_file(pp, filename);
    write_line_directive(pp);
}

static void if_part(struct preprocessor *pp)
{
    const_expression(pp);
    new_line(pp);
}

static void ifdef_part(struct preprocessor *pp)
{
    static char ident[128] = {'\0'};

    token(pp, ident);
    new_line(pp);

    if (!lookup_macro(pp->mactab, ident))
        pp->skip_depth++;
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
    else if (!strcmp(direc, "ifdef"))
        ifdef_part(pp);
    else if (!strcmp(direc, "ifndef"))
        ifndef_part(pp);
    else if (!strcmp(direc, "endif"))
        endif_line(pp);
    else
        unknown_directive(pp, direc);
}

static const char *next_arg(const char *args, char *buf)
{
    int depth = 0;
    const char *s = args;
    char *d = buf;

    for (;;) {
        if (*s == ',' || *s == '\0')
            break;

        if (*s == ')' && depth == 0)
            break;

        if (*s == '(')
            depth++;
        if (*s == ')')
            depth--;

        *d++ = *s++;
    }
    *d = '\0';
    return s;
}

static const char *read_args(const char *args, struct macro_param *params)
{
    struct macro_param *prm = params;
    char arg_[1024] = {'\0'};
    const char *s = args;

    while (*s != '(')
        s++;
    /* skip the first '(' */
    s++;

    for (;;) {
        if (!prm) {
            /* TODO error */
            return NULL;
        }

        s = next_arg(s, arg_);
        strbuf_copy(&prm->arg, arg_);
        prm = prm->next;

        if (*s == ')')
            /* the next char */
            return s + 1;
        else if (*s == ',') {
            s++;
            continue;
        }
        else if (*s == '\0')
            /* null char */
            return s;
    }
}

static const struct macro_param *find_param(const struct macro_param *params,
        const char *name)
{
    const struct macro_param *p;
    for (p = params; p; p = p->next)
        if (!strcmp(p->name, name))
            return p;
    return NULL;
}

static void glue(struct strbuf *dst, const char *src)
{
    strbuf_append(dst, src);
}

static void glue_ch(struct strbuf *dst, char src)
{
    strbuf_append_char(dst, src);
}

static void add_hideset(const struct macro_entry *mac,
        const struct macro_entry **hideset, int max_count)
{
    int i;

    if (!mac)
        return;

    for (i = 0; i < max_count; i++) {
        if (hideset[i] == mac)
            /* already in hideset */
            return;
        if (hideset[i] == NULL)
            /* end of list */
            break;
    }

    if (i == max_count)
        /* hideset is full. TODO error */
        return;

    hideset[i] = mac;
}

static void union_hideset(struct preprocessor *pp,
        const struct macro_entry **tmpset, int max_count)
{
    int i;

    for (i = 0; i < max_count; i++) {
        if (tmpset[i] == NULL)
            break;
        add_hideset(tmpset[i], pp->hideset, MAX_HIDESET);
    }
}

static int in_hideset(struct preprocessor *pp, const struct macro_entry *mac)
{
    int i;

    if (!mac)
        return 0;

    for (i = 0; i < MAX_HIDESET; i++) {
        if (pp->hideset[i] == mac)
            return 1;
        if (pp->hideset[i] == NULL)
            return 0;
    }

    return 0;
}

static const char *expand_fn(const struct macro_entry *mac,
        const char *args, struct strbuf *dst)
{
    char tok[128] = {'\0'};
    char *t = tok;
    const char *s = mac->repl;
    const char *end_of_args;

    end_of_args = read_args(args, mac->params);

    for (;;) {
        if (is_macroname(*s)) {
            /* making token */
            *t = *s;
            t++;
        } else {
            *t = '\0';
            if (t != tok) {
                /* end of token */
                const struct macro_param *prm = find_param(mac->params, tok);
                if (prm)
                    glue(dst, prm->arg.buf);
                else
                    glue(dst, tok);
            }
            t = tok;
            glue_ch(dst, *s);

            /* skip trailing spaces */
            if (is_whitespaces(*s))
                while (is_whitespaces(*(s + 1)))
                    s++;
        }

        /* need check after substitution, otherwise a token ending with
         * null char won't be substituted */
        if (!*s)
            break;
        s++;
    }

    return end_of_args;
}

static int subst(struct preprocessor *pp, const char *ts, struct strbuf *dst)
{
#define MAX_TMPSET 16
    const struct macro_entry *tmpset[MAX_TMPSET] = {NULL};
    struct macro_entry *mac;
    int has_subst = 0;
    char tok[128] = {'\0'};
    char *t = tok;
    const char *s = ts;

    for (;;) {
        if (is_macroname(*s)) {
            /* making token */
            *t = *s;
            t++;
        } else {
            *t = '\0';
            if (t != tok) {
                /* end of token */
                int in_hs;
                mac = lookup_macro(pp->mactab, tok);
                in_hs = in_hideset(pp, mac);

                if (mac && mac->repl && !in_hs) {
                    if (mac->is_func)
                        s = expand_fn(mac, s, dst);
                    else
                        glue(dst, mac->repl);

                    add_hideset(mac, tmpset, MAX_TMPSET);
                    has_subst = 1;
                } else {
                    glue(dst, tok);
                }
            }
            t = tok;
            glue_ch(dst, *s);
        }

        /* need check after substitution, otherwise a token ending with
         * null char won't be substituted */
        if (!*s)
            break;
        s++;
    }

    union_hideset(pp, tmpset, MAX_TMPSET);
    return has_subst;
}

static void expand_ts(struct preprocessor *pp, const char *ts)
{
    int nsubst = 0;
    struct strbuf result;
    strbuf_init(&result, 0);

    if (ts && ts[0] == '\0')
        return;

    nsubst = subst(pp, ts, &result);

    if (0) {
        static int depth = 0;
        printf("depth: [%d]\n", depth);
        printf("[%s]\n    => [%s] (%d)\n\n", ts, result.buf, nsubst);
        depth++;
    }

    if (nsubst)
        expand_ts(pp, result.buf);
    else
        writes(pp, result.buf);
    strbuf_free(&result);
}

static void glue_args(struct preprocessor *pp, struct strbuf *dst)
{
    int depth = 0;
    int done = 0;

    for (;;) {
        const int c = readc(pp);

        if (c == '(')
            depth++;

        if (c == ')')
            done = (--depth == 0);

        glue_ch(dst, c);

        if (done)
            return;

        if (!c) {
            /* TODO error */
            return;
        }
    }
}

static void expand(struct preprocessor *pp)
{
    static char tok[128] = {'\0'};
    struct macro_entry *mac;

    token(pp, tok);

    mac = lookup_macro(pp->mactab, tok);

    if (mac) {
        /* makes token sequence */
        struct strbuf ts;
        strbuf_init(&ts, 0);
        reset_hideset(pp);

        glue(&ts, tok);
        if (mac->is_func)
            glue_args(pp, &ts);

        expand_ts(pp, ts.buf);
        strbuf_free(&ts);
    }
    else {
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

int preprocess_file(struct preprocessor *pp, const char *filename)
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
