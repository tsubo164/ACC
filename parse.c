#include <stdlib.h>
#include "parse.h"

static struct ast_node *new_node(enum ast_node_kind kind,
        struct ast_node *l, struct ast_node *r)
{
    struct ast_node *n = malloc(sizeof(struct ast_node));
    n->kind = kind;
    n->l = l;
    n->r = r;
    n->value = 0;

    return n;
}

static enum token_kind gettok(struct parser *p)
{
    const int N = TOKEN_BUFFER_SIZE;

    if (p->head == p->curr) {
        p->head = (p->head + 1) % N;
        p->curr = (p->curr + 1) % N;
    } else {
        p->curr = (p->curr + 1) % N;
        return p->tokbuf[p->head].kind;
    }

    return lex_get_token(&p->lex, &p->tokbuf[p->head]);
}

static const struct token *gettok2(struct parser *p)
{
    const int N = TOKEN_BUFFER_SIZE;

    if (p->head == p->curr) {
        p->head = (p->head + 1) % N;
        p->curr = (p->curr + 1) % N;
    } else {
        p->curr = (p->curr + 1) % N;
        return &p->tokbuf[p->head];
    }

    lex_get_token(&p->lex, &p->tokbuf[p->head]);
    return &p->tokbuf[p->head];
}

static void ungettok(struct parser *p)
{
    const int N = TOKEN_BUFFER_SIZE;
    p->curr = (p->curr - 1 + N) % N;
}

/* XXX typedef TEST */
typedef struct token token;
typedef struct ast_node node;

static void error(struct parser *p, const char *msg)
{
    const struct token *tok = &p->tokbuf[p->head];
    p->error_pos = token_file_pos(tok);
    p->error_msg = msg;
}

static void expect(struct parser *p, enum token_kind query)
{
    const enum token_kind kind = gettok(p);
    const int N = TOKEN_BUFFER_SIZE;

    if (kind == query) {
        return;
    } else {
        /* XXX error handling */
        error(p, "error: unexpected token");
        p->curr = (p->curr - 1 + N) % N;
        return;
    }
}

void parser_init(struct parser *p)
{
    int i;

    for (i = 0; i < TOKEN_BUFFER_SIZE; i++) {
        token_init(&p->tokbuf[i]);
    }
    lexer_init(&p->lex);
    p->head = 0;
    p->curr = 0;

    p->error_pos = -1L;
    p->error_msg = "";
}

/*
 * forward declaration
 */
static struct ast_node *expression(struct parser *p);

/*
 * primary_expression
 *     : TK_NUM
 *     | TK_IDENT
 *     | '(' expression ')'
 *     ;
 */
static struct ast_node *primary_expression(struct parser *p)
{
    const struct token *tok = gettok2(p);
    node *nod = NULL;

    switch (tok->kind) {

    case TK_NUM:
        nod = new_node(NOD_NUM, NULL, NULL);
        nod->value = tok->value;
        return nod;

    case TK_IDENT:
        nod = new_node(NOD_VAR, NULL, NULL);
        nod->value = tok->value;
        return nod;

    case '(':
        nod = expression(p);
        expect(p, ')');
        return nod;

    default:
        ungettok(p);
        error(p, "unexpected token in expression");
        return NULL;
    }
}

/*
 * multiplicative_expression
 *     : primary_expression
 *     | multiplicative_expression '*' primary_expression
 *     | multiplicative_expression '/' primary_expression
 *     ;
 */
static struct ast_node *multiplicative_expression(struct parser *p)
{
    struct ast_node *base = primary_expression(p);

    for (;;) {
        const struct token *tok = gettok2(p);

        switch (tok->kind) {

        case '*':
            base = new_node(NOD_MUL, base, primary_expression(p));
            break;

        case '/':
            base = new_node(NOD_DIV, base, primary_expression(p));
            break;

        default:
            ungettok(p);
            return base;
        }
    }
}

/*
 * additive_expression
 *     : multiplicative_expression
 *     | additive_expression '+' multiplicative_expression
 *     | additive_expression '-' multiplicative_expression
 *     ;
 */
static struct ast_node *additive_expression(struct parser *p)
{
    struct ast_node *base = multiplicative_expression(p);

    for (;;) {
        const struct token *tok = gettok2(p);

        switch (tok->kind) {

        case '+':
            base = new_node(NOD_ADD, base, multiplicative_expression(p));
            break;

        case '-':
            base = new_node(NOD_SUB, base, multiplicative_expression(p));
            break;

        default:
            ungettok(p);
            return base;
        }
    }
}

/*
 * relational_expression
 *     : additive_expression
 *     | relational_expression '<' additive_expression
 *     | relational_expression '>' additive_expression
 *     | relational_expression TK_LE additive_expression
 *     | relational_expression TK_GE additive_expression
 *     ;
 */
static struct ast_node *relational_expression(struct parser *p)
{
    struct ast_node *base = additive_expression(p);

    for (;;) {
        const struct token *tok = gettok2(p);

        switch (tok->kind) {

        case '<':
            base = new_node(NOD_LT, base, additive_expression(p));
            break;

        case '>':
            base = new_node(NOD_GT, base, additive_expression(p));
            break;

        case TK_LE:
            base = new_node(NOD_LE, base, additive_expression(p));
            break;

        case TK_GE:
            base = new_node(NOD_GE, base, additive_expression(p));
            break;

        default:
            ungettok(p);
            return base;
        }
    }
}

/*
 * equality_expression
 *     : relational_expression
 *     | equality_expression TK_EQ relational_expression
 *     | equality_expression TK_NE relational_expression
 *     ;
 */
static struct ast_node *equality_expression(struct parser *p)
{
    struct ast_node *base = relational_expression(p);

    for (;;) {
        const struct token *tok = gettok2(p);

        switch (tok->kind) {

        case TK_EQ:
            base = new_node(NOD_EQ, base, relational_expression(p));
            break;

        case TK_NE:
            base = new_node(NOD_NE, base, relational_expression(p));
            break;

        default:
            ungettok(p);
            return base;
        }
    }
}

/*
 * assignment_expression
 *     : equality_expression
 *     | primary_expression '=' assignment_expression
 *     ;
 */
static struct ast_node *assignment_expression(struct parser *p)
{
    struct ast_node *base = equality_expression(p);

    /*
    for (;;) {
    */
        const struct token *tok = gettok2(p);

        switch (tok->kind) {

        case '=':
            base = new_node(NOD_ASSIGN, base, assignment_expression(p));
            break;

        default:
            ungettok(p);
            return base;
        }
    /*
    }
    */
            return base;
}

/*
 * expression
 *     : assignment_expression
 *     ;
 */
static struct ast_node *expression(struct parser *p)
{
    return assignment_expression(p);
}

/*
 * statement
 *     : expression ';'
 *     ;
 */
static struct ast_node *statement(struct parser *p)
{
    struct ast_node *base = expression(p);

    expect(p, ';');

    return base;
}

/*
 * statement_list
 *     : statement
 *     | statement_list statement
 *     ;
 */
static struct ast_node *statement_list(struct parser *p)
{
    struct ast_node *base = statement(p);

    for (;;) {
        const struct token *tok = gettok2(p);

        switch (tok->kind) {

        case TK_EOF:
            return base;

        default:
            ungettok(p);
            base = new_node(NOD_LIST, base, statement(p));
            break;
        }
    }
}

struct ast_node *parse(struct parser *p)
{
    return statement_list(p);
}
