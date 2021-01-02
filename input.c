int printf(char *s, int i);

enum color {
    R = 3,
    G,
    B
};

int main()
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
            printf("----- FOO %d\n", 110);
            break;
        case 111:
            break;
        case 112:
            break;
        case 113:
            break;
        case 114:
            break;
        }
        return a;
    case 13:
        return a;

    default:
        printf("----- HOGE %d\n", a);
        break;
    }

    return 42;
}
