#include "test.h"

int main()
{
    {
        int a = 0;
        goto X;
X:
        a++;
        /*
Y:
        */
        a++;
        /*
Z:
        */
        a++;
        assert(3, a);
    }
    {
        int a = 0;
        goto Q;
        /*
P:
        */
        a++;
Q:
        a++;
        /*
R:
        */
        a++;
        assert(2, a);
    }
    {
        int a = 0;
        goto K;
        /*
I:
        */
        a++;
        /*
J:
        */
        a++;
K:
        a++;
        assert(1, a);
    }

    return 0;
}
