#include "test.h"

#define ARRARY_SIZE 8

int main()
{
    {
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

    return 0;
}
