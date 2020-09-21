/* char data type */

int main()
{
    char x[2];
    int a;

    x[0] = 2;
    *(x + 1) = -7;

    x[0] = x[1] - x[0]; /* x[0] = -9 */

    a = 51;

    if (a + x[0] == 42)
        return 0;
    else
        return 1;
}
