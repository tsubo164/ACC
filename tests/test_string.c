int assert(int expected, int actual);

int main()
{
    {
        char *s = "abcdef";
        int i = 0;

        i = s[0];

        /* TODO implement convert char to int */
        assert(97, i);
    }

    return 0;
}
