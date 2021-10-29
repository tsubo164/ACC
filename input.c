/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

/* array in struct */
typedef struct foo {
    int a[4];
} Foo;

int main()
{
    Foo f = {{11, 22, 33, 44}};
    return f.a[2];
    /*
    return x + i;
    */
}

/*
thog;
*/

/*
int a = 0*
struct int 234
8 * >> {
}
*/

/*
void foo();
static int foo(void);

int main()
{
    int a = bar();

    return foo();
}

static int foo(void)
{
    return 42;
}
*/
/*
static struct ast_node *translation_unit(struct parser *p)
{
    static int null_count = 0;
    struct ast_node *tree = NULL;

    while (!consume(p, TOK_EOF)) {
        struct ast_node *decl = extern_decl(p);

        if (!decl) {
            if (++null_count >= 5)
                break;
        }

        tree = new_node(NOD_LIST, tree, decl);
    }

    return tree;
}
*/
