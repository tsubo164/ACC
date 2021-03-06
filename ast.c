#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "esc_seq.h"

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
    struct ast_node *n = calloc(1, sizeof(struct ast_node));
    n->kind = kind;
    n->l = l;
    n->r = r;

    n->type = type_void();

    return n;
}

void free_ast_node(struct ast_node *node)
{
    if (!node)
        return;

    free_ast_node(node->l);
    free_ast_node(node->r);
}

const char *node_to_string(const struct ast_node *node)
{
    if (node == NULL)
        return "null";

    switch (node->kind) {
    case NOD_NOP: return "NOD_NOP";
    case NOD_LIST: return "NOD_LIST";
    case NOD_COMPOUND: return "NOD_COMPOUND";
    case NOD_BREAK: return "NOD_BREAK";
    case NOD_CONTINUE: return "NOD_CONTINUE";
    case NOD_RETURN: return "NOD_RETURN";
    case NOD_GOTO: return "NOD_GOTO";
    case NOD_IF: return "NOD_IF";
    case NOD_IF_THEN: return "NOD_IF_THEN";
    case NOD_SWITCH: return "NOD_SWITCH";
    case NOD_CASE: return "NOD_CASE";
    case NOD_DEFAULT: return "NOD_DEFAULT";
    case NOD_LABEL: return "NOD_LABEL";
    case NOD_FOR: return "NOD_FOR";
    case NOD_FOR_PRE_COND: return "NOD_FOR_PRE_COND";
    case NOD_FOR_BODY_POST: return "NOD_FOR_BODY_POST";
    case NOD_WHILE: return "NOD_WHILE";
    case NOD_DOWHILE: return "NOD_DOWHILE";
    case NOD_ASSIGN: return "NOD_ASSIGN";
    case NOD_ADD_ASSIGN: return "NOD_ADD_ASSIGN";
    case NOD_SUB_ASSIGN: return "NOD_SUB_ASSIGN";
    case NOD_MUL_ASSIGN: return "NOD_MUL_ASSIGN";
    case NOD_DIV_ASSIGN: return "NOD_DIV_ASSIGN";
    case NOD_MOD_ASSIGN: return "NOD_MOD_ASSIGN";
    case NOD_SHL_ASSIGN: return "NOD_SHL_ASSIGN";
    case NOD_SHR_ASSIGN: return "NOD_SHR_ASSIGN";
    case NOD_OR_ASSIGN: return "NOD_OR_ASSIGN";
    case NOD_XOR_ASSIGN: return "NOD_XOR_ASSIGN";
    case NOD_AND_ASSIGN: return "NOD_AND_ASSIGN";
    case NOD_STRUCT_REF: return "NOD_STRUCT_REF";
    case NOD_CAST: return "NOD_CAST";
    case NOD_ADDR: return "NOD_ADDR";
    case NOD_DEREF: return "NOD_DEREF";
    case NOD_FUNC_DEF: return "NOD_FUNC_DEF";
    case NOD_CALL: return "NOD_CALL";
    case NOD_ARG: return "NOD_ARG";
    case NOD_ADD: return "NOD_ADD";
    case NOD_SUB: return "NOD_SUB";
    case NOD_MUL: return "NOD_MUL";
    case NOD_DIV: return "NOD_DIV";
    case NOD_MOD: return "NOD_MOD";
    case NOD_SHL: return "NOD_SHL";
    case NOD_SHR: return "NOD_SHR";
    case NOD_OR: return "NOD_OR";
    case NOD_XOR: return "NOD_XOR";
    case NOD_AND: return "NOD_AND";
    case NOD_NOT: return "NOD_NOT";
    case NOD_LT: return "NOD_LT";
    case NOD_GT: return "NOD_GT";
    case NOD_LE: return "NOD_LE";
    case NOD_GE: return "NOD_GE";
    case NOD_EQ: return "NOD_EQ";
    case NOD_NE: return "NOD_NE";
    case NOD_COMMA: return "NOD_COMMA";
    case NOD_COND: return "NOD_COND";
    case NOD_COND_THEN: return "NOD_COND_THEN";
    case NOD_LOGICAL_OR: return "NOD_LOGICAL_OR";
    case NOD_LOGICAL_AND: return "NOD_LOGICAL_AND";
    case NOD_LOGICAL_NOT: return "NOD_LOGICAL_NOT";
    case NOD_PREINC: return "NOD_PREINC";
    case NOD_PREDEC: return "NOD_PREDEC";
    case NOD_POSTINC: return "NOD_POSTINC";
    case NOD_POSTDEC: return "NOD_POSTDEC";
    case NOD_SIZEOF: return "NOD_SIZEOF";
    case NOD_IDENT: return "NOD_IDENT";
    case NOD_NUM: return "NOD_NUM";
    case NOD_FPNUM: return "NOD_FPNUM";
    case NOD_STRING: return "NOD_STRING";
    case NOD_CONST_EXPR: return "NOD_CONST_EXPR";
    case NOD_TYPE_NAME: return "NOD_TYPE_NAME";
    case NOD_DECL_IDENT: return "NOD_DECL_IDENT";
    case NOD_INIT: return "NOD_INIT";
    case NOD_DESIG: return "NOD_DESIG";
    default: return "**unknown**";
    }
}

static void print_type(const struct data_type *type)
{
    static char type_name[128] = {'\0'};

    make_type_name(type, type_name);
    printf(" %s", type_name);
}

static void print_tree_recursive(const struct ast_node *tree, int depth)
{
    int i;

    if (!tree)
        return;

    for (i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("%d. ", depth);

    if (tree->sym != NULL || tree->kind == NOD_NUM || tree->kind == NOD_FPNUM) {
        printf(TERMINAL_COLOR_CYAN);
        printf(TERMINAL_DECORATION_BOLD);
            printf("%s", node_to_string(tree));
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
    }
    else if (tree->kind == NOD_INIT) {
        printf(TERMINAL_COLOR_YELLOW);
        printf(TERMINAL_DECORATION_BOLD);
            printf("%s", node_to_string(tree));
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
    }
    else {
        printf("%s", node_to_string(tree));
    }

    /* print data type of node */
    printf(TERMINAL_COLOR_RED);
        print_type(tree->type);
    printf(TERMINAL_COLOR_RESET);

    if (tree->sym != NULL && tree->kind == NOD_DESIG) {
        printf(" ");
        printf(TERMINAL_COLOR_GREEN);
        printf(TERMINAL_DECORATION_BOLD);
        printf("%s", tree->sym->name);
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
    }

    switch (tree->kind) {

    case NOD_DECL_IDENT:
    case NOD_IDENT:
        printf(" ");
        printf(TERMINAL_COLOR_GREEN);
        printf(TERMINAL_DECORATION_BOLD);
        printf("%s", tree->sym->name);
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
        break;

    case NOD_DESIG:
    case NOD_INIT:
    case NOD_NUM:
    case NOD_CONST_EXPR:
        printf(TERMINAL_COLOR_MAGENTA);
        printf(TERMINAL_DECORATION_BOLD);
            printf(" %ld", tree->ival);
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
        break;

    case NOD_FPNUM:
        printf(TERMINAL_COLOR_MAGENTA);
        printf(TERMINAL_DECORATION_BOLD);
            printf(" %g", tree->fval);
        printf(TERMINAL_DECORATION_RESET);
        printf(TERMINAL_COLOR_RESET);
        break;

    case NOD_STRING:
        printf(TERMINAL_COLOR_MAGENTA);
        printf(TERMINAL_DECORATION_BOLD);
            printf(" \"");
            print_string_literal(stdout, tree->sym->name);
            printf("\"");
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
