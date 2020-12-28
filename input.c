int printf(char *s, int i);

int main()
{
    int i = 0;

    {
        while (1) {
            if (i / 3 == 4)
                if (i / 6 == 2)
                    break;
            i++;
        }
    }

    return i;
}
