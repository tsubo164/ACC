//#include <stdio.h>
//#include <string.h>

#define HASH_SIZE 1237 /* a prime number */

unsigned int dummy(unsigned int x)
{
    return x % HASH_SIZE;
}

int main()
{
    return dummy(139);
}
