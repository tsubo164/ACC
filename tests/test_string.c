#include "test.h"

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

    return 0;
}
