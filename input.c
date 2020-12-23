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
*/
enum color {
    R = 11,
    G/* = R + 23*/,
    B
};

int main()
{
    int a = 99;

    return a + R;
}
