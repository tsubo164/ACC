#include "test.h"
#include "gcc_func.h"

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

    return 0;
}
