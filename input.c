int main()
{
    int i = 0;

    for (;;)
        if (++i == 5)
            break;

    return i;
}
