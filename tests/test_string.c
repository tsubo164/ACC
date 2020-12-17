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

int main()
{
    char *s = "abcdef";
    int i = 0;

    i = s[0];

    /* TODO implement convert char to int */
    assert(97, i);

    return 0;
}
