enum color {
    R, G, B
};

typedef struct point {
    int x, y;
} Point;

int main()
{
        /* nested struct pointer with typedef */
        typedef struct node Node;

        struct node {
            int id;
            Node *next;
        };

        /*
        Node n0, n1, n2;

        n0.id = 123;
        n1.id = 765;
        n2.id = 999;

        n0.next = &n1;
        n1.next = &n2;
        n2.next = 0;

        assert(123, n0.id);
        assert(765, n0.next->id);
        assert(999, n0.next->next->id);
        assert(999, n1.next->id);
        */
    {
    }

    return sizeof Point;
}
