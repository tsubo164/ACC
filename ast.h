#ifndef AST_H
#define AST_H

#include "symbol.h"
#include "type.h"

enum ast_node_kind {
    NOD_GLOBAL,
    NOD_STMT,
    NOD_EXT,  /* to extend child nodes */
    NOD_IF,
    NOD_RETURN,
    NOD_WHILE,
    NOD_ASSIGN,
    NOD_STRUCT_DECL,
    NOD_MEMBER_DECL,
    NOD_STRUCT_REF,
    NOD_VAR,
    NOD_GLOBAL_VAR,
    NOD_VAR_DEF,
    NOD_ADDR,
    NOD_DEREF,
    NOD_CALL,
    NOD_FUNC_DEF,
    NOD_ARG,
    NOD_PARAM,
    NOD_NUM,
    NOD_ADD,
    NOD_SUB,
    NOD_MUL,
    NOD_DIV,
    NOD_LT,
    NOD_GT,
    NOD_LE,
    NOD_GE,
    NOD_EQ,
    NOD_NE
};

struct ast_node {
    enum ast_node_kind kind;
    const struct data_type *dtype;
    struct ast_node *l;
    struct ast_node *r;
    union {
        int ival;
        const struct symbol *sym;
    } data;
    const char *sval;
};

extern struct ast_node *new_ast_node(enum ast_node_kind kind,
        struct ast_node *l, struct ast_node *r);
extern void free_ast_node(struct ast_node *node);

extern void ast_node_set_symbol(struct ast_node *node, const struct symbol *sym);

extern const char *node_to_string(const struct ast_node *node);
extern void print_tree(const struct ast_node *tree);

#endif /* AST_H */
