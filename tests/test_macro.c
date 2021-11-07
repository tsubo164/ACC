#include "test.h"

#define ARRARY_SIZE 8
#define ADD(x, y) ((x) + (y))
#define SQUARE(x) ((x) * (x))

enum {
    A = 123,
    B = 456,
    C = 789,
    D = 111
};
/* cyclic define */
#define A B
#define B C
#define C A
#define D 999

/* empty macro replacements */
#define va_end(ap)
#define NOTHING

/* testing hideset for one macro apearing twice */
#define SEVEN (7)
#define OP(op) (SEVEN op SEVEN)

int main()
{
    {
        /* object-like macros */
        int a[ARRARY_SIZE] = {11, 22, 33, 44};

        assert(32, sizeof a);
        assert(8, (sizeof a) / sizeof (a[0]));
        assert(11, a[0]);
        assert(22, a[1]);
        assert(33, a[2]);
        assert(44, a[3]);
        assert(0,  a[4]);
        assert(0,  a[5]);
        assert(0,  a[6]);
        assert(0,  a[7]);
    }
    {
        /* function-like macros */
        int x = ADD(39, 3);
        int y = 4;

        assert(42, x);
        assert(20, SQUARE(y++));
        assert(6, y);
    }
    {
        /* cyclic define */
        assert(123, A);
        assert(456, B);
        assert(789, C);
        assert(999, D);
    }
    {
        /* empty macro replacements */
        /*
        int a = 42;

        va_end(a);
        NOTHING;

        assert(123, a);
        */
    }
    {
        int a = OP(+);

        assert(14, a);
        assert(49, OP(*));
    }

    return 0;
}
