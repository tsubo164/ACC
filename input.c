int printf(char *s, int i);

int a = 12;

int main()
{
        /*
    int i;

    for (i = 0; i < 10; i++)
        ;
        printf("i -> %d\n", i);
        */

    /*
    int a = 5;

    if (a > 3)
        goto error;
    else
        goto final;

    */
error:
    printf("**** error\n", 0);

final:
    printf("**** error\n", 0);

    {
        int a = 12;

        switch (a) {
        case 1:
            return a;
        case 2:
            break;
        case 12:
            switch (110) {
            case 110:
                break;
            case 111:
                break;
            case 12:
                break;
            case 13:
                break;
            case 112:
                break;
            }
            break;
            return a;

        case 13:
            return a;

        default:
            break;
        }
    }
    /*
    goto foo;
    {
error:
    }
    */

    return 42;
}
