/*
#include <stdio.h>
*/

/*
#include <stdio.h>

void foo(float f)
{
    printf("%f\n", f);
}

void bar(int i)
{
    printf("%d\n", i);
}
*/

/*
enum {A, B, C};

enum operand_size { X, Y, Z};
const int  size_to_offset[] = {0, 1, 2, 3, 4, 3, 4};
*/

void assert(int i, int j)
{
}
int main()
{
    /*
    int array[3];

    double g = 3.14;
    foo(g);

    array[C] = 19;

    return array[C];
    */
    /*
    enum operand_size size = Y;

    return size_to_offset[size];
    */
    typedef int Int;
    Int i = 32;
    int *p;

    p = i;
    {
        /* enum with no tag */
        enum {
            red = 101, green, blue
        } col;

        col = green;

        assert(102, col);

        p = col;
        p = green;
    }
    return 0;
}
