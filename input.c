//void puts(char *s);

struct {
    int x, y;
} no_tag;

int main()
{
    no_tag.x = 123;
    return no_tag.x;
}
