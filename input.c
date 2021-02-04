int main()
{
    int i = 13;
    int *p;
    int **pp;
    int ***ppp;
    int a[3][2];

    p = &i;
    pp = &p;
    ppp = &pp;

    //*p = 41;
    //**pp = 41;
    ***ppp = 41;

    return ***ppp;
}