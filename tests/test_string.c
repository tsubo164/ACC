#include "test.h"
/*
TODO fix infinite loop. make it enable to include std headers from here
#include "../include/stdio.h"
#include "../include/string.h"
*/
int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int strcmp(const char *s1, const char *s2);

enum {
    A,
    B,
    C
};

/* string literal with the same name of other symbols */
const char *enum_to_str(int kind)
{
    switch (kind) {
    case A: return "A"; break;
    case B: return "B"; break;
    case C: return "C"; break;
    default: break;
    }
    return "NULL";
}

int main()
{
    {
        /* basic of string literal */
        char *s = "abcdef";
        int i = 0;

        i = s[0];

        assert(97, i);
        assert(97, s[0]);
    }
    {
        /* size of string literal */
        assert(14, sizeof "Hello, world\n");
        assert(15, sizeof "He\tllo, world\n");
    }
    {
        /* string literal with the same name of other symbols */
        int kind = 0;
        const char *s = enum_to_str(kind);

        assert('A', s[0]);
        assert('\0', s[1]);
    }
    {
        /* string literal with escape sequence \x */
        const char *s = "Hello\x0a";

        assert('H', s[0]);
        assert('e', s[1]);
        assert('l', s[2]);
        assert('l', s[3]);
        assert('o', s[4]);
        assert('\n', s[5]);
        assert(10, s[5]);
    }
    {
        /* string literal with double quotation mark */
        const char *str = "this is /* string \"literal\"\n";
        char buf[128];

        buf[0] = 't';
        buf[1] = '\0';
        sprintf(buf, "%s", str);

        assert(1, !strcmp(buf, str));
        assert(34, buf[18]);
        assert(34, buf[26]);
    }

    return 0;
}
