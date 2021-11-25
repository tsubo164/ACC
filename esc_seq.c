#include <stdio.h>
#include <ctype.h>
#include "esc_seq.h"

int char_to_escape_sequence(int ch, char *es)
{
    int es1 = '\0';

    if (!es)
        return 0;

    switch (ch) {
    case '\0': es1 = '0';  break;
    case '\\': es1 = '\\'; break;
    case '\'': es1 = '\''; break;
    case '"':  es1 = '"';  break;
    case '\a': es1 = 'a';  break;
    case '\b': es1 = 'b';  break;
    case '\f': es1 = 'f';  break;
    case '\n': es1 = 'n';  break;
    case '\r': es1 = 'r';  break;
    case '\t': es1 = 't';  break;
    case '\v': es1 = 'v';  break;
    default:
       return 0;
    }

    es[0] = '\\';
    es[1] = es1;
    es[2] = '\0';

    return 1;
}

void print_string_literal(FILE *fp, const char *src)
{
    const char *s = NULL;
    char es[4] = {'\0'};

    for (s = src; *s; s++) {
        if (iscntrl(*s))
            fprintf(fp, "\\%03o", *s);
        else if (char_to_escape_sequence(*s, es) && *s != '\'')
            fprintf(fp, "%s", es);
        else
            fprintf(fp, "%c", *s);
    }
}

void make_string_literal(const char *src, char *dst, size_t n)
{
    const char *s = NULL;
    char *d = dst;
    char es[4] = {'\0'};
    const int max = n - 1;

    for (s = src; *s; s++) {
        if (iscntrl(*s)) {
            if (d - dst > max - 4)
                break;
            sprintf(d, "\\%03o", *s);
            d += 4;
        }
        else if (char_to_escape_sequence(*s, es) && *s != '\'') {
            if (d - dst > max - 2)
                break;
            sprintf(d, "\\%c", es[1]);
            d += 2;
        }
        else {
            if (d - dst > max - 1)
                break;
            *d++ = *s;
        }
    }

    *d = '\0';
}
