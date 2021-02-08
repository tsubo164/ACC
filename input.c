void printf(char *s, int n);

typedef struct node Node;

struct node {
    int id;
    Node *next;
};

int main()
{
    Node n0, n1, n2;
    Node *n;

    n0.id = 123;
    n1.id = 765;
    n2.id = 999;

    n0.next = &n1;
    n1.next = &n2;
    n2.next = 0;

    for (n = &n0; n; n = n->next)
        printf("node.id: %d\n", n->id);

    return n0.id;
}
