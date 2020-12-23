int printf(char *s, int a, int b);
int exit(int code);

int assert(int expected, int actual)
{
    if (expected != actual) {
        printf("error: expected: %d actual: %d\n", expected, actual);
        exit(1);
    }
    return 0;
}
