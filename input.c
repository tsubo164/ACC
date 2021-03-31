#include <stdio.h>
#include <stdlib.h>

#define HASH_SIZE 1237 /* a prime number */
#define MULTIPLIER 31

static unsigned int hash_fn(const char *key)
{
    unsigned int h = 0;
    unsigned const char *p = NULL;

    for (p = (unsigned const char *) key; *p != '\0'; p++)
        h = MULTIPLIER * h + *p;

    return h % HASH_SIZE;
}

int main()
{
    size_t i = 123;
    char s[5] = {'t', 'h', 's', 't', '\0'};

    unsigned int h = hash_fn(s);

    char *p = malloc(12 * sizeof(char));
    p[0] = 'G';
    p[1] = 'o';
    p[2] = '!';
    p[3] = '\0';

    h = hash_fn(p);

    puts(p);
    puts(s);
    printf(" => %d\n", h);

    free(p);

    return sizeof i;
}
