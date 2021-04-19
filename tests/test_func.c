#include "test.h"
#include "gcc_func.h"

/* function with 8 parameters compiled by acc */
int sum1234_mult_sum5678(
        int a1, int a2, int a3, int a4,
        int a5, int a6, int a7, int a8)
{
    return
        (a1 + a2 + a3 + a4) *
        (a5 + a6 + a7 + a8);
}

int main()
{
    {
        assert(42, gcc_twice(21));
        assert(16, gcc_twice(gcc_twice(4)));
    }
    {
        long a = 0;
        gcc_add(11, 22, 33, 44, &a);

        assert(110, a);
    }
    {
        /* function with 8 parameters compiled by gcc */
        int a = gcc_sum1234_mult_sum5678(1, 2, 3, 4, 5, 6, 7, 8);

        assert(260, a);
    }
    {
        /* function with 8 parameters compiled by acc */
        int a = sum1234_mult_sum5678(1, 2, 3, 4, 5, 6, 7, 8);

        assert(260, a);
    }

    return 0;
}
