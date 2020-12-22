#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

#define TERMINAL_COLOR_BLACK   "\x1b[30m"
#define TERMINAL_COLOR_RED     "\x1b[31m"
#define TERMINAL_COLOR_GREEN   "\x1b[32m"
#define TERMINAL_COLOR_YELLOW  "\x1b[33m"
#define TERMINAL_COLOR_BLUE    "\x1b[34m"
#define TERMINAL_COLOR_MAGENTA "\x1b[35m"
#define TERMINAL_COLOR_CYAN    "\x1b[36m"
#define TERMINAL_COLOR_WHITE   "\x1b[37m"
#define TERMINAL_COLOR_RESET   "\x1b[39m"

#define TERMINAL_DECORATION_BOLD    "\x1b[1m"
#define TERMINAL_DECORATION_RESET   "\x1b[0m"

struct ast_node *new_ast_node(enum ast_node_kind kind,
        struct ast_node *l, struct ast_node *r)
{
    struct ast_node *n = malloc(sizeof(struct ast_node));
    n->kind = kind;
    n->l = l;
    n->r = r;

    n->type = type_void();

    n->ival = 0;
    n->sval = NULL;

    return n;
}

void free_ast_node(struct ast_node *node)
{
    if (!node)
        return;

    free_ast_node(node->l);
    free_ast_node(node->r);
}

#define AST_NODE_LIST(N) \
    N(NOD_LIST) \
    N(NOD_COMPOUND) \
    N(NOD_STMT) \
    N(NOD_IF) \
    N(NOD_THEN) \
    N(NOD_RETURN) \
    N(NOD_WHILE) \
    N(NOD_ASSIGN) \
    N(NOD_STRUCT_REF) \
    N(NOD_ADDR) \
    N(NOD_DEREF) \
    N(NOD_FUNC_DEF) \
    N(NOD_CALL) \
    N(NOD_ARG) \
    N(NOD_ADD) \
    N(NOD_SUB) \
    N(NOD_MUL) \
    N(NOD_DIV) \
    N(NOD_LT) \
    N(NOD_GT) \
    N(NOD_LE) \
    N(NOD_GE) \
    N(NOD_EQ) \
    N(NOD_NE) \
    N(NOD_IDENT) \
    N(NOD_NUM) \
    N(NOD_STRING) \
    N(NOD_DECL) \
    N(NOD_DECLARATOR) \
    N(NOD_DECL_DIRECT) \
    N(NOD_DECL_INIT) \
    N(NOD_DECL_IDENT) \
    N(NOD_DECL_FUNC) \
    N(NOD_DECL_PARAM) \
    N(NOD_DECL_MEMBER) \
    N(NOD_SPEC_CHAR) \
    N(NOD_SPEC_INT) \
    N(NOD_SPEC_POINTER) \
    N(NOD_SPEC_ARRAY) \
    N(NOD_SPEC_STRUCT)

const char *node_to_string(const struct ast_node *node)
{
    if (node == NULL)
        return "null";

#define N(kind) case kind: return #kind;
    switch (node->kind) {
AST_NODE_LIST(N)
    default: return "**unknown**";
    }
#undef N
}

static void print_tree_recursive(const struct ast_node *tree, int depth)
{
    int i;
    for (i = 0; i < depth; i++) {
        printf("  ");
    }

    if (!tree) {
        printf("(null)\n");
        return;
    }

    if (tree->sym != NULL || tree->kind == NOD_NUM) {
        printf(TERMINAL_COLOR_CYAN);
        printf(TERMINAL_DECORATION_BOLD);
            printf("%s", node_to_string(tree));
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
    }
    else if (tree->kind >= NOD_SPEC_CHAR) {
        printf(TERMINAL_COLOR_RED);
        printf(TERMINAL_DECORATION_BOLD);
            printf("%s", node_to_string(tree));
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
    }
    else {
        printf("%s", node_to_string(tree));
    }

    switch (tree->kind) {

    case NOD_DECL_IDENT:
    case NOD_IDENT:
        printf(" ");
        printf(TERMINAL_COLOR_GREEN);
        printf(TERMINAL_DECORATION_BOLD);
        printf("%s", tree->sval);
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
        break;

    case NOD_NUM:
        printf(TERMINAL_COLOR_MAGENTA);
        printf(TERMINAL_DECORATION_BOLD);
            printf(" %d", tree->ival);
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
        break;

    case NOD_STRING:
        printf(TERMINAL_COLOR_MAGENTA);
        printf(TERMINAL_DECORATION_BOLD);
            printf(" \"%s\"", tree->sval);
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
        break;

    default:
        break;
    }
    printf("\n");

    print_tree_recursive(tree->l, depth + 1);
    print_tree_recursive(tree->r, depth + 1);
}

void print_tree(const struct ast_node *tree)
{
    print_tree_recursive(tree, 0);
}

static void print_decl_recursive(const struct ast_node *tree)
{
    if (!tree)
        return;

    switch (tree->kind) {

    case NOD_DECL:
        printf("declaration: ");
        break;

    case NOD_DECL_FUNC:
        printf(" function(");
        print_decl_recursive(tree->l);
        printf(") returning");
        print_decl_recursive(tree->r);
        return;

    case NOD_SPEC_ARRAY:
        if (tree->ival > 0)
            printf(" array %d of", tree->ival);
        else
            printf(" array of");
        break;

    case NOD_SPEC_POINTER:
        printf(" pointer to");
        break;

    case NOD_SPEC_CHAR:
        printf(" char");
        break;

    case NOD_SPEC_INT:
        printf(" int");
        break;

    case NOD_SPEC_STRUCT:
        printf(" struct %s", tree->sval);
        break;

    case NOD_DECL_IDENT:
        printf("%s is", tree->sval);
        break;

    default:
        break;
    }

    print_decl_recursive(tree->l);
    print_decl_recursive(tree->r);
}

void print_decl(const struct ast_node *tree)
{
    print_decl_recursive(tree);
    printf("\n");
}
