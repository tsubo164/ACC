#ifndef PARSE_H
#define PARSE_H

#include "lexer.h"

#define TOKEN_BUFFER_SIZE 2

struct parser {
    struct lexer lex;
    struct token tokbuf[TOKEN_BUFFER_SIZE];
    int head, curr;

    long error_pos;
    const char *error_msg;
};

enum ast_node_kind {
    NOD_LIST, /* for list */
    NOD_EXT,  /* to extend child nodes */
    NOD_IF,
    NOD_RETURN,
    NOD_WHILE,
    NOD_ASSIGN,
    NOD_VAR,
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
    struct ast_node *l;
    struct ast_node *r;
    int value;

    /*
    union {
        struct {
            int value;
        } int_;

        struct {
            int offset;
        } var;

        struct {
            struct ast_node *operand;
        } una;

        struct {
            struct ast_node *left;
            struct ast_node *right;
        } bin;

        struct {
            struct ast_node *cond;
            struct ast_node *then;
            struct ast_node *else_;
        } if_;

        struct {
            struct ast_node *init;
            struct ast_node *cond;
            struct ast_node *post;
            struct ast_node *body;
        } for_;

        struct {
            struct ast_node *cond;
            struct ast_node *body;
        } while_;
    } op;
    */
};

extern void parser_init(struct parser *p);

extern struct ast_node *parse(struct parser *p);

#if 0
    node->op.If.cond;
    node->op.if_.cond = cond;
    set_if_cond(node, cond);

    node->op.if_.cond;
    get_if_cond(node);
    node->OP_IF_COND;

/* unary */
extern void set_unary_operand(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_unary_operand(struct ast_node *n);

/* binary */
extern void set_binary_left(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_binary_left(struct ast_node *n, struct ast_node *op);

extern void set_right(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_binary_right(struct ast_node *n);

/* if then else */
extern void set_if_cond(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_if_cond(struct ast_node *n);

extern void set_if_then(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_if_then(struct ast_node *n);

extern void set_if_else(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_if_else(struct ast_node *n);

/* for */
extern void set_for_init(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_for_init(struct ast_node *n);

extern void set_for_cond(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_for_cond(struct ast_node *n);

extern void set_for_post(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_for_post(struct ast_node *n);

extern void set_for_body(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_for_body(struct ast_node *n);

/* while */
extern void set_while_cond(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_while_cond(struct ast_node *n);

extern void set_while_body(struct ast_node *n, struct ast_node *op);
extern struct ast_node *get_while_body(struct ast_node *n);

/* unary */
#define OPERAND l
/* binary */
#define LEFT l
#define RIGHT r
/* if then else */
#define COND l
#define THEN r
#define ELSE data.c0
/* for/while */
#define INIT r
#define POST data.c0
#define BODY data.c1
#endif

#endif /* _H */
