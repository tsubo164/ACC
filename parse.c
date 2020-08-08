#include <stdlib.h>
#include <string.h>
#include "parse.h"

struct symbol {
    char *name;
    int offset;
};

static struct symbol symtbl[32] = {{NULL, 0}};
static int nsyms = 0;

static const struct symbol *lookup_symbol(const char *name)
{
    struct symbol *sym = NULL;
    int i;

    for (i = 0; i < nsyms; i++)
    {
        sym = &symtbl[i];

        if (!strcmp(sym->name, name))
        {
            return sym;
        }
    }

    sym = &symtbl[nsyms++];

    sym->name = malloc(strlen(name) + 1);
    strcpy(sym->name, name);
    sym->offset = 8 * nsyms;

    return sym;
}

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

static const struct token *gettok(struct parser *p)
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

static const struct token *current_token(const struct parser *p)
{
    return &p->tokbuf[p->curr];
}

static void ungettok(struct parser *p)
{
    const int N = TOKEN_BUFFER_SIZE;
    p->curr = (p->curr - 1 + N) % N;
}

static void error(struct parser *p, const char *msg)
{
    const struct token *tok = current_token(p);
    p->error_pos = token_file_pos(tok);
    p->error_msg = msg;
}

static void expect(struct parser *p, enum token_kind query)
{
    const struct token *tok = gettok(p);

    if (tok->kind == query) {
        return;
    } else {
        ungettok(p);
        error(p, "unexpected token after this");
        return;
    }
}

static void expect_or_error(struct parser *p, enum token_kind query, const char *error_msg)
{
    const struct token *tok = gettok(p);

    if (tok->kind == query) {
        return;
    } else {
        ungettok(p);
        error(p, error_msg);
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
    const struct token *tok = gettok(p);
    struct ast_node *base = NULL;

    switch (tok->kind) {

    case TK_NUM:
        /* ??? base = new_number(tok->value); */
        base = new_node(NOD_NUM, NULL, NULL);
        base->value = tok->value;
        return base;

    case TK_IDENT:
        base = new_node(NOD_VAR, NULL, NULL);
        {
            const struct symbol *sym = lookup_symbol(tok->word);
            base->value = sym->offset;
        }
        return base;

    case '(':
        base = expression(p);
        expect(p, ')');
        return base;

    default:
        ungettok(p);
        error(p, "unexpected token in expression");
        return NULL;
    }
}

/*
 * unary_expression
 *     : '+' primary_expression
 *     | '-' primary_expression
 *     ;
 */
static struct ast_node *unary_expression(struct parser *p)
{
    const struct token *tok = gettok(p);
    struct ast_node *base = NULL;

    switch (tok->kind) {

    case '+':
        base = primary_expression(p);
        return base;

    case '-':
        /* ??? base = new_number(-1); */
        base = new_node(NOD_NUM, NULL, NULL);
        base->value = -1;
        base = new_node(NOD_MUL, base, primary_expression(p));
        return base;

    default:
        ungettok(p);
        base = primary_expression(p);
        return base;
    }
}

/*
 * multiplicative_expression
 *     : unary_expression
 *     | multiplicative_expression '*' unary_expression
 *     | multiplicative_expression '/' unary_expression
 *     ;
 */
static struct ast_node *multiplicative_expression(struct parser *p)
{
    struct ast_node *base = unary_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '*':
            base = new_node(NOD_MUL, base, unary_expression(p));
            break;

        case '/':
            base = new_node(NOD_DIV, base, unary_expression(p));
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
        const struct token *tok = gettok(p);

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
        const struct token *tok = gettok(p);

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
        const struct token *tok = gettok(p);

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
        const struct token *tok = gettok(p);

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
 *     | TK_RETURN expression ';'
 *     ;
 */
static struct ast_node *statement(struct parser *p)
{
    struct ast_node *base;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TK_RETURN:
        base = new_node(NOD_RETURN, expression(p), NULL);
        break;

    default:
        ungettok(p);
        base = expression(p);
        break;
    }

    /*
    expect(p, ';');
    */
    expect_or_error(p, ';', "missing ';' at end of statement");

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
        const struct token *tok = gettok(p);

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
