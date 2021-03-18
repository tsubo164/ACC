#include "input.h"

int main()
{
    struct foo {
        const unsigned int ui, uj;
    } foo = {29, 31};

    return foo.ui + foo.uj;
}
