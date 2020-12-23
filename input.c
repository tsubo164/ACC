/*
int add(int a, int b);

char x = 29;
struct point;
struct point {
    int x, y;
};
*/

/*
enum color;
int i = 0;
*/

enum color {
    R = 10 + 1 + 12,
    G = R + 100 - 50 / 2,
    B
};

int main()
{
    int a = 99;

    return a + R;
}
