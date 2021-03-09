int main()
{
    /*
    int a[2][2] = {{11, 22}, {33, 44}};

    return a[0][1];
    */
    struct point {
        int x, y;
    } line[2] = {{11, 22}, {33, 44}};

    return line[1].x + sizeof(line);
}
