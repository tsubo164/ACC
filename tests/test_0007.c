/* initialization for local and global variables */
int x = 8;
char y;

int num()
{
    /* returns 8 */
    return x;
}

int main()
{
    char a = 31;
    int b = 3 + a; /* b = 34 */

    if (b + num() + y == 42)
        return 0;
    else
        return 1;
}
