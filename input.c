int count_up()
{
    struct point {
        int x, y;
    } static pt = {0};

    pt.x++;

    return pt.x;
}

int main()
{
    int static const i = 123;
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
