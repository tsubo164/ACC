/*
struct vec {
    int x, y;
};
*/

enum size {
    ARRAY_SIZE = 3,
    B
};

int main()
{
    int a[2 + 2];// = {1, 2, 3, 42};
    //int b[ARRAY_SIZE];// = {1, 2, 3, 42};

    a[3] = 42;
    //b[2] = 19;

    return a[3];// + b[2];
}
