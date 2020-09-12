#include <stdio.h>
#include "semantics.h"

static void promote_data_type2(struct ast_node *node)
{
    const struct ast_node *n1, *n2;

    if (!node) {
        return;
    }

    n1 = node->l;
    n2 = node->r;

    if (!n1 && !n2) {
        return;
    }

    if (!n1) {
        node->dtype = n2->dtype;
        return;
    }

    if (!n2) {
        node->dtype = n1->dtype;
        return;
    }

    /*
    printf("    [%s]\n", node_to_string(n1));
    printf("            =><%p>\n", (void *)n1->l);
    printf("            =><%p>\n", (void *)n1->r);
    printf("            =><%p>\n", (void *)n1->dtype);
    printf("            =><%p>\n", (void *)n2->dtype);
    */

    if (n1->dtype->kind > n2->dtype->kind) {
        node->dtype = n1->dtype;
        return;
    } else {
        node->dtype = n2->dtype;
        return;
    }
}
#if 0
#endif

static const struct data_type *promote_data_type(
        const struct ast_node *n1, const struct ast_node *n2)
{
    if (!n1 && !n2) {
        return type_void();
    }

    if (!n1) {
        return n2->dtype;
    }

    if (!n2) {
        return n1->dtype;
    }

    /*
    printf("    [%s]\n", node_to_string(n1));
    printf("            =><%p>\n", (void *)n1->l);
    printf("            =><%p>\n", (void *)n1->r);
    printf("            =><%p>\n", (void *)n1->dtype);
    printf("            =><%p>\n", (void *)n2->dtype);
    */

    if (n1->dtype->kind > n2->dtype->kind) {
        return n1->dtype;
    } else {
        return n2->dtype;
    }
}

static int promote_type(struct ast_node *node)
{
    if (!node) {
        return 0;
    }

    /* void */
    /*
    if (node->dtype == NULL) {
        node->dtype = type_void();
    }
    */

    /*
    printf("    [%s]\n", node_to_string(node));
    printf("            => [%s]\n", data_type_to_string(node->dtype));
    printf("            => [%p]\n", (void *)node->dtype);
    printf("            => [%p]\n", (void *)node->l);
    printf("            => [%p]\n", (void *)node->r);
    */
    /*
    */


    promote_type(node->l);
    promote_type(node->r);

    /* promote */
    /*
    promote_data_type2(node);
    printf("    [%s]\n", node_to_string(node));
    printf("            => [%s]\n", data_type_to_string(node->dtype));
    node->dtype = promote_data_type(node->l, node->r);
    printf("    <%s>\n", node_to_string(node));
    printf("            => <%s>\n", data_type_to_string(node->dtype));
    */

    /*
    if (node->data.sym) {
        node->dtype = node->data.sym->dtype;
        return 0;
    }

    if (node->dtype == NULL) {
        node->dtype = type_void();
        return 0;
    }
    node->dtype = promote_data_type(node->l, node->r);
    */

    /*
    */
    switch (node->kind) {

    case NOD_ADD:
        node->dtype = promote_data_type(node->l, node->r);
        /*
        */

        if (node->l->dtype->kind == DATA_TYPE_ARRAY) {
            struct ast_node *size, *mul;

            size = new_ast_node(NOD_NUM, NULL, NULL);
            size->data.ival = node->l->dtype->ptr_to->byte_size;
            mul = new_ast_node(NOD_MUL, size, node->r);
            node->r = mul;
        } else {
            /*
            node->dtype = promote_data_type(node->l, node->r);
            */
        }
        break;

    case NOD_SUB:
    case NOD_MUL:
    case NOD_DIV:
    case NOD_ASSIGN:
        /*
    case NOD_EQ:
        */
        node->dtype = promote_data_type(node->l, node->r);
        /*
        printf("    [%s]\n", data_type_to_string(node->dtype));
        printf("        [%s]\n", data_type_to_string(node->l->dtype));
        printf("            <%d>\n", node->l->dtype->kind);
        printf("        [%s]\n", data_type_to_string(node->r->dtype));
        printf("            <%d>\n", node->r->dtype->kind);
        */
        break;

        /*
    case NOD_NUM:
        break;
        */

        /*
    case NOD_VAR:
        node->dtype = node->data.sym->dtype;
        break;
        */

    case NOD_DEREF:
        node->dtype = promote_data_type(node->l, node->r);
        /*
        */
        node->dtype = node->dtype->ptr_to;;
        break;
#if 0
#endif

    default:
        /*
        if (node->dtype == NULL) {
            node->dtype = type_void();
        }
        */
        /*
        else if (node->data.sym) {
            printf("--------%d\n", node->data.sym->kind);
            node->dtype = node->data.sym->dtype;
        }
        */
        break;
    }

#if 0
    printf("    [%s]\n", data_type_to_string(node->dtype));
    printf("        <%d>\n", node->dtype->kind);
    /*
    */
#endif

    /*
    printf("    [%s]\n", node_to_string(node));
    printf("            => [%s]\n", data_type_to_string(node->dtype));
    */

    return 0;
}

static int promote_type__(struct ast_node *node)
{
    if (!node) {
        return 0;
    }

    promote_type__(node->l);
    promote_type__(node->r);

    switch (node->kind) {

    case NOD_ADD:
    case NOD_SUB:
    case NOD_MUL:
    case NOD_DIV:
    case NOD_ASSIGN:

    case NOD_DEREF:
        node->dtype = promote_data_type(node->l, node->r);
        break;

    default:
        break;
    }

    /*
    promote_data_type2(node);
    printf("    [%s]\n", node_to_string(node));
    printf("            => [%s]\n", data_type_to_string(node->dtype));
    */

    return 0;
}
int semantic_analysis(struct ast_node *tree, struct symbol_table *table)
{
    /*
    */
    int promo;

    /*
    promo = promote_type__(tree);
    */

    promo = promote_type(tree);

    return 0;
}
