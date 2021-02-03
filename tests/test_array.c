int assert(int expected, int actual);

/* array index access */
int main()
{
    {
        int a[4];

        a[0] = 3;
        a[1] = 7;

        a[2] = a[0] + a[1];
        a[3] = a[2] * 4 + 2;

        assert(10, a[2]);
        assert(42, a[3]);
        assert(42, *(a+3));
        assert(16, sizeof a);
        assert(4, sizeof a[2]);
    }

    return 0;
}
