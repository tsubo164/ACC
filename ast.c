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
    struct position pos = {0};
    struct ast_node *n = malloc(sizeof(struct ast_node));
    n->kind = kind;
    n->l = l;
    n->r = r;

    n->type = type_void();
    n->sym = NULL;

    n->ival = 0;
    n->sval = NULL;
    n->pos = pos;

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
    case NOD_STRUCT_REF: return "NOD_STRUCT_REF";
    case NOD_ADDR: return "NOD_ADDR";
    case NOD_DEREF: return "NOD_DEREF";
    case NOD_FUNC_DEF: return "NOD_FUNC_DEF";
    case NOD_CALL: return "NOD_CALL";
    case NOD_ARG: return "NOD_ARG";
    case NOD_ADD: return "NOD_ADD";
    case NOD_SUB: return "NOD_SUB";
    case NOD_MUL: return "NOD_MUL";
    case NOD_DIV: return "NOD_DIV";
    case NOD_LT: return "NOD_LT";
    case NOD_GT: return "NOD_GT";
    case NOD_LE: return "NOD_LE";
    case NOD_GE: return "NOD_GE";
    case NOD_EQ: return "NOD_EQ";
    case NOD_NE: return "NOD_NE";
    case NOD_NOT: return "NOD_NOT";
    case NOD_COND: return "NOD_COND";
    case NOD_COND_THEN: return "NOD_COND_THEN";
    case NOD_LOGICAL_OR: return "NOD_LOGICAL_OR";
    case NOD_LOGICAL_AND: return "NOD_LOGICAL_AND";
    case NOD_PREINC: return "NOD_PREINC";
    case NOD_PREDEC: return "NOD_PREDEC";
    case NOD_POSTINC: return "NOD_POSTINC";
    case NOD_POSTDEC: return "NOD_POSTDEC";
    case NOD_SIZEOF: return "NOD_SIZEOF";
    case NOD_IDENT: return "NOD_IDENT";
    case NOD_NUM: return "NOD_NUM";
    case NOD_STRING: return "NOD_STRING";
    case NOD_CONST_EXPR: return "NOD_CONST_EXPR";
    case NOD_TYPE_NAME: return "NOD_TYPE_NAME";
    case NOD_DECL: return "NOD_DECL";
    case NOD_DECL_SPEC: return "NOD_DECL_SPEC";
    case NOD_DECLARATOR: return "NOD_DECLARATOR";
    case NOD_DECL_DIRECT: return "NOD_DECL_DIRECT";
    case NOD_DECL_INIT: return "NOD_DECL_INIT";
    case NOD_DECL_IDENT: return "NOD_DECL_IDENT";
    case NOD_DECL_FUNC: return "NOD_DECL_FUNC";
    case NOD_DECL_PARAM: return "NOD_DECL_PARAM";
    case NOD_DECL_MEMBER: return "NOD_DECL_MEMBER";
    case NOD_DECL_ENUMERATOR: return "NOD_DECL_ENUMERATOR";
    case NOD_DECL_TYPEDEF: return "NOD_DECL_TYPEDEF";
    case NOD_DECL_EXTERN: return "NOD_DECL_EXTERN";
    case NOD_DECL_STATIC: return "NOD_DECL_STATIC";
    case NOD_SPEC_VOID: return "NOD_SPEC_VOID";
    case NOD_SPEC_CHAR: return "NOD_SPEC_CHAR";
    case NOD_SPEC_INT: return "NOD_SPEC_INT";
    case NOD_SPEC_POINTER: return "NOD_SPEC_POINTER";
    case NOD_SPEC_ARRAY: return "NOD_SPEC_ARRAY";
    case NOD_SPEC_STRUCT: return "NOD_SPEC_STRUCT";
    case NOD_SPEC_ENUM: return "NOD_SPEC_ENUM";
    case NOD_SPEC_TYPE_NAME: return "NOD_SPEC_TYPE_NAME";
    default: return "**unknown**";
    }
}

static void print_type(const struct data_type *type)
{
    printf(" ");
    if (is_type_name(type)) {
        printf("%s", type_name_of(type));
    }
    else if (is_struct(type)) {
        printf("struct %s", type_name_of(type));
    }
    else if (is_enum(type)) {
        printf("enum %s", type_name_of(type));
    }
    else if (is_array(type)) {
        printf("[%d]", get_array_length(type));
        print_type(underlying(type));
    }
    else if (is_pointer(type)) {
        printf("*");
        print_type(underlying(type));
    }
    else {
        printf("%s", type_name_of(type));
    }
}

static void print_tree_recursive(const struct ast_node *tree, int depth)
{
    int i;
    for (i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("%d. ", depth);

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
    else if (tree->kind >= NOD_SPEC_VOID) {
        printf(TERMINAL_COLOR_RED);
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
            printf(" \"%s\"", tree->sym->name);
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

    case NOD_SPEC_VOID:
        printf(" void");
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

    case NOD_SPEC_ENUM:
        printf(" enum %s", tree->sval);
        break;

    case NOD_SPEC_TYPE_NAME:
        printf(" %s", tree->sval);
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
