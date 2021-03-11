#include "test.h"

int foo()
{
    int i = 0;

    while (i < 10) {
        i = 2 * i + 1;
    }

    assert(15, i);
    return i;
}

int num()
{
    int n;
    n = 7;
    return n;
}

int add(int x, int y)
{
    return x + y;
}

int main()
{
    {
        int a;
        int b;
        int c;

        a = add(21, 7) +  2 * num();
        assert(42, a);

        b = 42 / num() < 7;
        assert(1, b);

        c = foo() > 14;
        assert(1, c);
        assert(44, a + b + c);
    }
    {
        int sum = 0;
        int i = 0;

        while (i < 10) {
            i++;
            sum = sum + i;
        }

        assert(55, sum);
    }
    {
        int sum = 0;
        int i = 0;

        do {
            i++;
            sum = sum + i;
        } while (i < 10);

        assert(55, sum);
    }
    {
        int i = 0;

        do ++i; while (i < 10);

        assert(10, i);
    }
    {
        int i = 0;

        while (i < 10)
            if (i++ == 6)
                break;
        assert(7, i);
    }
    {
        int i = 0;

        while (1) {
            if (i / 3 == 4)
                if (i / 6 == 2)
                    break;
            i++;
        }
        assert(12, i);
    }
    {
        int sum = 0;
        int i = 0;

        while (i < 20) {
            if (i / 3 == 3) {
                sum += i;
                i++;
                continue;
            }
            i++;
        }
        assert(30, sum);
    }
    {
        int sum = 0;
        int i = 0;

        do {
            if (i / 3 == 3) {
                sum += i;
                i++;
                continue;
            }
            i++;
        } while (i < 20);
        assert(30, sum);
    }

    return 0;
}
