#ifndef AST_H
#define AST_H

#include "symbol.h"
#include "type.h"

enum ast_node_kind {
    /* list */
    NOD_LIST,
    NOD_COMPOUND,
    /* statement */
    NOD_STMT,
    NOD_RETURN,
    NOD_BREAK,
    NOD_IF,
    NOD_IF_THEN,
    NOD_WHILE,
    NOD_DOWHILE,
    NOD_FOR,
    NOD_FOR_PRE_COND,
    NOD_FOR_BODY_POST,
    /* expression */
    NOD_ASSIGN,
    NOD_ADD_ASSIGN,
    NOD_SUB_ASSIGN,
    NOD_MUL_ASSIGN,
    NOD_DIV_ASSIGN,
    NOD_STRUCT_REF,
    NOD_ADDR,
    NOD_DEREF,
    NOD_FUNC_DEF,
    NOD_CALL,
    NOD_ARG,
    NOD_ADD,
    NOD_SUB,
    NOD_MUL,
    NOD_DIV,
    NOD_LT,
    NOD_GT,
    NOD_LE,
    NOD_GE,
    NOD_EQ,
    NOD_NE,
    NOD_PREINC,
    NOD_PREDEC,
    NOD_POSTINC,
    NOD_POSTDEC,
    /* primary */
    NOD_IDENT,
    NOD_NUM,
    NOD_STRING,
    /* declaration */
    NOD_DECL,
    NOD_DECLARATOR,
    NOD_DECL_DIRECT,
    NOD_DECL_INIT,
    NOD_DECL_IDENT,
    NOD_DECL_FUNC,
    NOD_DECL_PARAM,
    NOD_DECL_MEMBER,
    NOD_DECL_ENUMERATOR,
    /* type specifier */
    NOD_SPEC_CHAR,
    NOD_SPEC_INT,
    NOD_SPEC_POINTER,
    NOD_SPEC_ARRAY,
    NOD_SPEC_STRUCT,
    NOD_SPEC_ENUM
};

struct ast_node {
    enum ast_node_kind kind;
    const struct data_type *type;
    const struct symbol *sym;

    struct ast_node *l;
    struct ast_node *r;

    int ival;
    const char *sval;
};

extern struct ast_node *new_ast_node(enum ast_node_kind kind,
        struct ast_node *l, struct ast_node *r);
extern void free_ast_node(struct ast_node *node);

extern const char *node_to_string(const struct ast_node *node);
extern void print_tree(const struct ast_node *tree);
extern void print_decl(const struct ast_node *tree);

#endif /* _H */
