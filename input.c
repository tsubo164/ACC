int count_up()
{
    /*
    struct point {
        int x, y;
    } static pt = {0};

    pt.x++;

    return pt.x;
    */
    int static i = 0;
    return ++i;
}

int main()
{
    int const static i = 123;
    /*
    struct point {
        int x, y;
    } static pt;

    pt.x = 123;
    */

    count_up();
    count_up();
    count_up();
    count_up();
    return count_up() + i;
}
