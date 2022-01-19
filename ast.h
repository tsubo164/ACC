#ifndef AST_H
#define AST_H

#include "position.h"
#include "symbol.h"
#include "type.h"

enum ast_node_kind {
    NOD_NOP,
    /* list */
    NOD_LIST,
    NOD_COMPOUND,
    /* statement */
    NOD_BREAK,
    NOD_CONTINUE,
    NOD_RETURN,
    NOD_GOTO,
    NOD_IF,
    NOD_IF_THEN,
    NOD_SWITCH,
    NOD_CASE,
    NOD_DEFAULT,
    NOD_LABEL,
    NOD_FOR,
    NOD_FOR_PRE_COND,
    NOD_FOR_BODY_POST,
    NOD_WHILE,
    NOD_DOWHILE,
    /* expression */
    NOD_ASSIGN,
    NOD_ADD_ASSIGN,
    NOD_SUB_ASSIGN,
    NOD_MUL_ASSIGN,
    NOD_DIV_ASSIGN,
    NOD_MOD_ASSIGN,
    NOD_SHL_ASSIGN,
    NOD_SHR_ASSIGN,
    NOD_OR_ASSIGN,
    NOD_XOR_ASSIGN,
    NOD_AND_ASSIGN,
    NOD_STRUCT_REF,
    NOD_CAST,
    NOD_ADDR,
    NOD_DEREF,
    NOD_FUNC_DEF,
    NOD_CALL,
    NOD_ARG,
    NOD_ADD,
    NOD_SUB,
    NOD_MUL,
    NOD_DIV,
    NOD_MOD,
    NOD_SHL,
    NOD_SHR,
    NOD_OR,
    NOD_XOR,
    NOD_AND,
    NOD_NOT,
    NOD_LT,
    NOD_GT,
    NOD_LE,
    NOD_GE,
    NOD_EQ,
    NOD_NE,
    NOD_COMMA,
    NOD_COND,
    NOD_COND_THEN,
    NOD_LOGICAL_OR,
    NOD_LOGICAL_AND,
    NOD_LOGICAL_NOT,
    NOD_PREINC,
    NOD_PREDEC,
    NOD_POSTINC,
    NOD_POSTDEC,
    NOD_SIZEOF,
    /* primary */
    NOD_IDENT,
    NOD_NUM,
    NOD_STRING,
    NOD_CONST_EXPR,
    NOD_TYPE_NAME,
    /* declaration */
    NOD_DECL_IDENT,
    NOD_DECL_PARAM,
    NOD_DECL_MEMBER,
    NOD_DECL_ENUMERATOR,
    NOD_DECL_TYPEDEF,
    NOD_DECL_EXTERN,
    NOD_DECL_STATIC,
    /* initializer */
    NOD_INIT,
    NOD_DESIG,
    /* type qualifier */
    NOD_QUAL_CONST,
    /* type specifier */
    NOD_SPEC_VOID,
    NOD_SPEC_CHAR,
    NOD_SPEC_SHORT,
    NOD_SPEC_INT,
    NOD_SPEC_LONG,
    NOD_SPEC_SIGNED,
    NOD_SPEC_UNSIGNED,
    NOD_SPEC_POINTER,
    NOD_SPEC_ARRAY,
    NOD_SPEC_STRUCT,
    NOD_SPEC_UNION,
    NOD_SPEC_ENUM,
    NOD_SPEC_TYPE_NAME,
    NOD_SPEC_ELLIPSIS
};

struct ast_node {
    enum ast_node_kind kind;
    struct data_type *type;
    struct symbol *sym;

    struct ast_node *l;
    struct ast_node *r;

    long ival;
    struct position pos;
};

extern struct ast_node *new_ast_node(enum ast_node_kind kind,
        struct ast_node *l, struct ast_node *r);
extern void free_ast_node(struct ast_node *node);

extern const char *node_to_string(const struct ast_node *node);
extern void print_tree(const struct ast_node *tree);
extern void print_decl(const struct ast_node *tree);

#endif /* _H */
