/*
#include <stdio.h>
*/

typedef struct {
    void *p;
    int i;
} Foo[1];

/* array parameter overrides type objects with pointer type */
void foo(Foo f);
/*
*/

typedef int Int;

int main() {
    typedef struct node Node;

    struct node {
        int id;
        Node *next;
        /*
        struct node *next;
        */
    };

    Node n0 = {9};
    /* typedef creates another data_type object */
    /*
    struct node n0 = {9};
    */
    Node n1;

    n0.next->id = 42;;

    return sizeof(Foo);
}
