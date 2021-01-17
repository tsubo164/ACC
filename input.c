int printf(char *s, int i);

struct vec {
    int x, y, z;
};

int main()
{
    //struct point p;
    struct vec v;
    //struct vec *pv;

    /*
    pv = &v;
    pv->x = 51;
    */

    v.x = 42;
    // *pv = 119;


    return v.x;
}
/*
*/
