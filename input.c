enum {
    E = 2
};

int main()
{
    int a = 42;

    switch (3) {

    case 19:
        a = 3;
        break;

    case E + 1:
        a = 4;
        break;

    default:
        break;
    }

    return a;
}
