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

struct preprocessor *new_preprocessor()
{
    struct preprocessor *pp;

    pp = (struct preprocessor *) malloc(sizeof(struct preprocessor));

    pp->text = (struct strbuf *) malloc(sizeof(struct strbuf));
    strbuf_init(pp->text);
    pp->filename = NULL;
    pp->y = 1;
    pp->x = 0;
    pp->prevx = 0;

    return pp;
}

void free_preprocessor(struct preprocessor *pp)
{
    if (!pp)
        return;

    strbuf_free(pp->text);
    free(pp->text);
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

static void writec(struct preprocessor *pp, char c)
{
    strbuf_append_char(pp->text, c);
}

static void writes(struct preprocessor *pp, const char *c)
{
    strbuf_append(pp->text, c);
}

static void write_line_directive(struct preprocessor *pp)
{
    static char buf[256] = {'\0'};

    sprintf(buf, "# %d \"%s\"\n", pp->y, pp->filename);
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

static void whitespaces(struct preprocessor *pp)
{
    int c;
    for (;;) {
        c = readc(pp);
        if (!is_whitespaces(c))
            break;
    }
    unreadc(pp, c);
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
        } else if (c == EOF) {
            error_(pp, "unterminated /* comment");
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
        if (isalnum(c)) {
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

static void file_path(struct preprocessor *pp, char *buf)
{
    char *p = buf;
    int c, quot;

    whitespaces(pp);

    c = readc(pp);
    if (c == '"') {
        quot = '"';
    } else if (c == '<') {
        quot = '>';
    } else {
        goto file_path_error;
    }

    for (;;) {
        const int c = readc(pp);
        if (isalnum(c) || c == '_' || c == '.' || c == '-') {
            *p++ = c;
        } else {
            unreadc(pp, c);
            break;
        }
    }
    *p = '\0';

    c = readc(pp);
    if (c != quot)
        goto file_path_error;

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
    static char filename[128] = {'\0'};

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

static void endif_line(struct preprocessor *pp)
{
    new_line(pp);
}

static void define_line(struct preprocessor *pp)
{
    static char ident[128] = {'\0'};
    static char repl[1024] = {'\0'};

    whitespaces(pp);
    token(pp, ident);

    token_list(pp, repl);
    new_line(pp);

    printf("#define %s [%s]\n", ident, repl);
}

static void non_directive(struct preprocessor *pp)
{
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

    if (!strcmp(direc, "include"))
        include_line(pp);
    else if (!strcmp(direc, "if"))
        if_part(pp);
    else if (!strcmp(direc, "endif"))
        endif_line(pp);
    else if (!strcmp(direc, "define"))
        define_line(pp);
    else {
        writes(pp, "# ");
        writes(pp, direc);
        non_directive(pp);
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
                block_comment(pp);
                continue;
            }
            else {
                unreadc(pp, c1);
            }
        }
        else if (c == '#') {
            directive_line(pp);
            continue;
        }
        else if (c == EOF) {
            break;
        }

        writec(pp, c);
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
