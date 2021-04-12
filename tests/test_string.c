#include "test.h"

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
    default: "NULL"; break;
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

    return 0;
}
