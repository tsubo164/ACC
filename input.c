#include "input.h"

int main()
{
    puts("=========================");
    {
        unsigned char uc = 250;
        if (uc < 0)
            printf("uc => %u < 0\n", uc);
    }
    puts("=========================");
    {
        char uc = 250;
        if (uc < 0)
            printf("uc => %d < 0\n", uc);
    }

    return 0;
}
