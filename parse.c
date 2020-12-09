#include <stdlib.h>
#include <string.h>
#include "parse.h"

/* XXX */
#define NEW_(kind) new_node(kind, NULL, NULL)
static struct ast_node *new_node(enum ast_node_kind kind,
        struct ast_node *l, struct ast_node *r)
{
    return new_ast_node(kind, l, r);
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

static void copy_token_text(struct parser *p, struct ast_node *node)
{
    const struct token *tok = current_token(p);
    node->sval = tok->text;
}

static void copy_token_ival(struct parser *p, struct ast_node *node)
{
    const struct token *tok = current_token(p);
    node->data.ival = tok->value;
}

static const struct token *consume(struct parser *p, int token_kind)
{
    const struct token *tok = gettok(p);

    if (tok->kind != token_kind) {
        ungettok(p);
        return NULL;
    }

    return tok;
}

/*
 * XXX
 * error_at(struct parser *p, long file_pos, const char *msg)
 */
static void error(struct parser *p, const char *msg)
{
    const struct token *tok = current_token(p);
    p->error_pos = token_file_pos(tok);
    p->error_msg = msg;

    printf("!!error %ld: %s\n", p->error_pos, msg);
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
        error(p, error_msg);
        return;
    }
}

struct parser *new_parser()
{
    struct parser *p = malloc(sizeof(struct parser));
    if (!p) {
        return NULL;
    }

    parser_init(p);
    return p;
}

void free_parser(struct parser *p)
{
    if (!p)
        return;
    free(p);
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

    init_symbol_table(&p->symtbl);
}

/*
 * forward declarations
 */
static struct ast_node *expression(struct parser *p);
static struct ast_node *assignment_expression(struct parser *p);
static struct ast_node *type_spec(struct parser *p);
static struct ast_node *decl_spec(struct parser *p);
static struct ast_node *declarator(struct parser *p);
static struct ast_node *declaration_list(struct parser *p);
static struct ast_node *statement_list(struct parser *p);

/*
 * identifier
 *     TOK_IDENT
 */
static struct ast_node *identifier(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = consume(p, TOK_IDENT);

    if (!tok)
        return NULL;

    tree = new_node(NOD_IDENT, NULL, NULL);
    tree->sval = tok->text;

    return tree;
}

/*
 * primary_expression
 *     TOK_NUM
 *     TOK_IDENT
 *     '(' expression ')'
 */
static struct ast_node *primary_expression(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_NUM:
        tree = new_node(NOD_NUM, NULL, NULL);
        /* TODO convert in semantics */
        tree->data.ival = tok->value;
        tree->sval = tok->text;
        return tree;

    case TOK_IDENT:
        ungettok(p);
        return identifier(p);

    case '(':
        tree = expression(p);
        expect(p, ')');
        return tree;

    default:
        /* TODO * when parser expects an expression, does it accept
         * null expressions * or treat them as an error?  */
        ungettok(p);
        return NULL;
    }
}

/*
 * argument_expression
 *     assignment_expression
 */
static struct ast_node *argument_expression(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *asgn = NULL;

    asgn = assignment_expression(p);
    if (!asgn)
        return NULL;

    tree = new_node(NOD_ARG, NULL, NULL);
    tree->l = asgn;

    return tree;
}

/*
 * argument_expression_list
 *     argument_expression_list
 *     argument_expression_list ',' argument_expression
 */
static struct ast_node *argument_expression_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *arg = argument_expression(p);

        if (!arg)
            return tree;

        tree = new_node(NOD_LIST, tree, arg);

        if (!consume(p, ','))
            return tree;
    }
}

/*
 * postfix_expression
 *     primary_expression
 *     postfix_expression '.' TOK_IDENT
 *     postfix_expression '(' argument_expression_list ')'
 */
static struct ast_node *postfix_expression(struct parser *p)
{
    struct ast_node *tree = primary_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '.':
            tree = new_node(NOD_STRUCT_REF, tree, identifier(p));
            break;

        case '(':
            tree = new_node(NOD_CALL, tree, NULL);
            tree->r = argument_expression_list(p);
            expect(p, ')');
            break;

        case '[':
            tree = new_node(NOD_ADD, tree, expression(p));
            expect(p, ']');
            tree = new_node(NOD_DEREF, tree, NULL);
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * unary_expression
 *     '+' primary_expression
 *     '-' primary_expression
 *     '*' unary_expression
 *     '&' unary_expression
 */
static struct ast_node *unary_expression(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case '+':
        return postfix_expression(p);

    case '-':
        /* TODO tree = new_number(-1); */
        tree = new_node(NOD_NUM, NULL, NULL);
        tree->data.ival = -1;
        tree = new_node(NOD_MUL, tree, postfix_expression(p));
        return tree;

    case '*':
        return new_node(NOD_DEREF, postfix_expression(p), NULL);

    case '&':
        return new_node(NOD_ADDR, postfix_expression(p), NULL);

    default:
        ungettok(p);
        return postfix_expression(p);
    }
}

/*
 * multiplicative_expression
 *     unary_expression
 *     multiplicative_expression '*' unary_expression
 *     multiplicative_expression '/' unary_expression
 */
static struct ast_node *multiplicative_expression(struct parser *p)
{
    struct ast_node *tree = unary_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '*':
            tree = new_node(NOD_MUL, tree, unary_expression(p));
            break;

        case '/':
            tree = new_node(NOD_DIV, tree, unary_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * additive_expression
 *     multiplicative_expression
 *     additive_expression '+' multiplicative_expression
 *     additive_expression '-' multiplicative_expression
 */
static struct ast_node *additive_expression(struct parser *p)
{
    struct ast_node *tree = multiplicative_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '+':
            tree = new_node(NOD_ADD, tree, multiplicative_expression(p));
            break;

        case '-':
            tree = new_node(NOD_SUB, tree, multiplicative_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * relational_expression
 *     additive_expression
 *     relational_expression '<' additive_expression
 *     relational_expression '>' additive_expression
 *     relational_expression TOK_LE additive_expression
 *     relational_expression TOK_GE additive_expression
 */
static struct ast_node *relational_expression(struct parser *p)
{
    struct ast_node *tree = additive_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '<':
            tree = new_node(NOD_LT, tree, additive_expression(p));
            break;

        case '>':
            tree = new_node(NOD_GT, tree, additive_expression(p));
            break;

        case TOK_LE:
            tree = new_node(NOD_LE, tree, additive_expression(p));
            break;

        case TOK_GE:
            tree = new_node(NOD_GE, tree, additive_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * equality_expression
 *     relational_expression
 *     equality_expression TOK_EQ relational_expression
 *     equality_expression TOK_NE relational_expression
 */
static struct ast_node *equality_expression(struct parser *p)
{
    struct ast_node *tree = relational_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_EQ:
            tree = new_node(NOD_EQ, tree, relational_expression(p));
            break;

        case TOK_NE:
            tree = new_node(NOD_NE, tree, relational_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * assignment_expression
 *     equality_expression
 *     primary_expression '=' assignment_expression
 */
static struct ast_node *assignment_expression(struct parser *p)
{
    struct ast_node *tree = equality_expression(p);

    /*
    for (;;) {
    */
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '=':
            tree = new_node(NOD_ASSIGN, tree, assignment_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    /*
    }
    */
            return tree;
}

/*
 * expression
 *     assignment_expression
 */
static struct ast_node *expression(struct parser *p)
{
    return assignment_expression(p);
}

/*
 * compound_statement
 *     '{' '}'
 *     '{' statement_list '}'
 *     '{' declaration_list '}'
 *     '{' declaration_list statement_list '}'
 */
static struct ast_node *compound_statement(struct parser *p)
{
    struct ast_node *tree = NULL;

    expect(p, '{');

    tree = new_node(NOD_COMPOUND, NULL, NULL);
    tree->l = declaration_list(p);
    tree->r = statement_list(p);

    expect(p, '}');

    return tree;
}

/*
 * statement
 *     expression ';'
 *     TOK_RETURN expression ';'
 */
static struct ast_node *statement(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_RETURN:
        tree = new_node(NOD_RETURN, expression(p), NULL);
        expect_or_error(p, ';', "missing ';' at end of return statement");
        break;

    case TOK_IF:
        expect_or_error(p, '(', "missing '(' after if");
        tree = new_node(NOD_IF, expression(p), NULL);
        expect_or_error(p, ')', "missing ')' after if condition");
        tree->r = new_node(NOD_THEN, statement(p), NULL);
        tok = gettok(p);
        if (tok->kind == TOK_ELSE) {
            tree->r->r = statement(p);
        } else {
            ungettok(p);
        }
        break;

    case TOK_WHILE:
        expect_or_error(p, '(', "missing '(' after while");
        tree = new_node(NOD_WHILE, expression(p), NULL);
        expect_or_error(p, ')', "missing ')' after while condition");
        tree->r = statement(p);
        break;

    case '{':
        ungettok(p);
        return compound_statement(p);

    case '}':
        ungettok(p);
        return NULL;

    default:
        ungettok(p);
        tree = expression(p);
        expect_or_error(p, ';', "missing ';' at end of statement");
        break;
    }

    return tree;
}

static struct ast_node *decl_ident(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = consume(p, TOK_IDENT);

    if (!tok)
        return tree;

    tree = NEW_(NOD_DECL_IDENT);
    tree->sval = tok->text;

    return tree;
}

/*
 * struct_declarator
 *     declarator
 *     ':' constant_expression
 *     declarator ':' constant_expression
 */
static struct ast_node *struct_declarator(struct parser *p)
{
    struct ast_node *tree = NEW_(NOD_DECLARATOR);

    tree->l = declarator(p);

    return tree;
}

/*
 * struct_declarator_list
 *     struct_declarator
 *     struct_declarator_list ',' struct_declarator
 */
static struct ast_node *struct_declarator_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *sdecl = struct_declarator(p);

        if (!sdecl)
            return tree;

        tree = new_node(NOD_LIST, tree, sdecl);

        if (!consume(p, ','))
            return tree;
    }
}

/*
 * struct_decl
 *     spec_qual_list struct_declarator_list
 */
static struct ast_node *struct_decl(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *spec = NULL;

    spec = type_spec(p);
    if (!spec)
        return NULL;

    tree = NEW_(NOD_DECL_MEMBER);
    tree->l = spec;
    tree->r = struct_declarator_list(p);

    return tree;
}

/*
 * struct_decl_list
 *     struct_decl
 *     struct_decl_list ',' struct_decl
 */
static struct ast_node *struct_decl_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *decl = struct_decl(p);

        if (!decl)
            return tree;

        tree = new_node(NOD_LIST, tree, decl);
        expect(p, ';');
    }
}

static struct ast_node *struct_union(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_STRUCT:
        tree = NEW_(NOD_SPEC_STRUCT);
        break;

    default:
        /* TODO error */
        break;
    }
    return tree;
}

static struct ast_node *struct_union_spec(struct parser *p)
{
    struct ast_node *tree = NULL;

    tree = struct_union(p);
    tree->l = decl_ident(p);

    if (!consume(p, '{')) {
        return tree;
    }

    tree->r = struct_decl_list(p);
    expect(p, '}');

    return tree;
}

static struct ast_node *type_spec(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_CHAR:
        tree = NEW_(NOD_SPEC_CHAR);
        break;

    case TOK_INT:
        tree = NEW_(NOD_SPEC_INT);
        break;

    case TOK_STRUCT:
        ungettok(p);
        tree = struct_union_spec(p);
        break;

    default:
        ungettok(p);
        break;
    }

    return tree;
}

static struct ast_node *param_decl(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *spec = NULL;

    spec = decl_spec(p);
    if (!spec)
        return NULL;

    tree = NEW_(NOD_DECL_PARAM);
    tree->l = spec;
    tree->r = declarator(p);

    return tree;
}

/*
 * param_decl_list
 *     param_decl
 *     param_decl_list ',' param_decl
 */
static struct ast_node *param_decl_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *param = param_decl(p);

        if (!param)
            return tree;

        tree = new_node(NOD_LIST, tree, param);

        if (!consume(p, ','))
            return tree;
    }
}

static struct ast_node *direct_declarator(struct parser *p)
{
    struct ast_node *tree = NULL;

    tree = NEW_(NOD_DECL_DIRECT);

    if (consume(p, '(')) {
        tree->l = declarator(p);
        expect(p, ')');
    }

    if (consume(p, TOK_IDENT)) {
        struct ast_node *ident = NEW_(NOD_DECL_IDENT);
        copy_token_text(p, ident);
        tree->r = ident;
    }

    if (consume(p, '[')) {
        struct ast_node *array = NEW_(NOD_SPEC_ARRAY);

        if (consume(p, TOK_NUM))
            copy_token_ival(p, array);

        tree->l = array;
        expect(p, ']');
    }

    if (consume(p, '(')) {
        struct ast_node *fn = NEW_(NOD_DECL_FUNC);
        fn->l = tree;
        fn->r = param_decl_list(p);
        tree = fn;
        expect(p, ')');
    }

    return tree;
}

static struct ast_node *pointer(struct parser *p)
{
    struct ast_node *tree = NULL;

    while (consume(p, '*'))
        tree = new_node(NOD_SPEC_POINTER, tree, NULL);

    return tree;
}

static struct ast_node *declarator(struct parser *p)
{
    struct ast_node *tree = NEW_(NOD_DECLARATOR);

    tree->l = pointer(p);
    tree->r = direct_declarator(p);

    return tree;
}

static struct ast_node *initializer(struct parser *p)
{
    struct ast_node *tree = assignment_expression(p);
    return tree;
}

static struct ast_node *init_declarator(struct parser *p)
{
    struct ast_node *tree = NEW_(NOD_DECL_INIT);

    tree->l = declarator(p);

    if (consume(p, '='))
        tree->r = initializer(p);

    return tree;
}

/*
 * init_declarator_list
 *     init_declarator
 *     init_declarator_list ',' init_declarator
 */
static struct ast_node *init_declarator_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *init = init_declarator(p);

        if (!init)
            return tree;

        tree = new_node(NOD_LIST, tree, init);

        if (!consume(p, ','))
            return tree;
    }
}

static struct ast_node *decl_spec(struct parser *p)
{
    struct ast_node *tree = type_spec(p);
    return tree;
}

static struct ast_node *declaration(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *spec = NULL;

    spec = decl_spec(p);
    if (!spec)
        return NULL;

    tree = NEW_(NOD_DECL);
    tree->l = spec;
    tree->r = init_declarator_list(p);

    if (consume(p, '{')) {
        /* TODO remove this by making peek() */
        ungettok(p);
        tree = new_node(NOD_FUNC_DEF, tree, compound_statement(p));
        return tree;
    }

    if (consume(p, ';'))
        return tree;

    return tree;
}

/*
 * declaration_list
 *     declaration
 *     declaration_list declaration
 */
static struct ast_node *declaration_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *decl = declaration(p);

        if (!decl)
            return tree;

        tree = new_node(NOD_LIST, tree, decl);
    }
}

/*
 * statement_list
 *     statement
 *     statement_list statement
 */
static struct ast_node *statement_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *stmt = statement(p);

        if (!stmt)
            return tree;

        tree = new_node(NOD_LIST, tree, stmt);
    }
}

static struct ast_node *extern_decl(struct parser *p)
{
    return declaration(p);
}

static struct ast_node *translation_unit(struct parser *p)
{
    struct ast_node *tree = NULL;

    while (!consume(p, TOK_EOF)) {
        tree = new_node(NOD_LIST, tree, extern_decl(p));
    }

    return tree;
}

struct ast_node *parse(struct parser *p)
{
    return translation_unit(p);
}
