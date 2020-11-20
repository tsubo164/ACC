#include <stdlib.h>
#include <string.h>
#include "parse.h"

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

/* TODO rename to next */
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
    struct parser *p = /*(struct parser *)*/ malloc(sizeof(struct parser));
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
static struct ast_node *statement(struct parser *p);
static struct ast_node *assignment_expression(struct parser *p);

/*
 * identifier
 *     : TOK_IDENT
 *     ;
 */
static struct ast_node *identifier(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = consume(p, TOK_IDENT);

    if (!tok)
        return NULL;

    /* TODO this could use NOD_MEMBER
     * this contains semantics of identifier but
     * function call uses NOD_CALL as well.
     * try to be consistent whichever to be chosen
     */
    tree = new_node(NOD_VAR, NULL, NULL);
    tree->sval = tok->text;

    return tree;
}

/*
 * identifier
 *     : TOK_IDENT
 *     ;
 */
static struct ast_node *identifier2(struct parser *p)
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
 *     : TOK_NUM
 *     | TOK_IDENT
 *     | '(' expression ')'
 *     ;
 */
static struct ast_node *primary_expression(struct parser *p)
{
    struct ast_node *tree = NULL;
    /*
    const char *ident = NULL;
    */
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_NUM:
        /* TODO tree = new_number(tok->value); */
        tree = new_node(NOD_NUM, NULL, NULL);
        /* TODO convert in semantics */
        tree->data.ival = tok->value;
        tree->sval = tok->text;
        return tree;

    case TOK_IDENT:
        ungettok(p);
        return identifier2(p);
#if 0
        ident = tok->text;

        tok = gettok(p);
        if (/*tok->kind == '('*/ 0) {
            /* parameters for function call */
            for (;;) {
                struct ast_node *expr = expression(p);
                if (expr == NULL) {
                    break;
                }

                tree = new_node(NOD_ARG, tree, expr);
                tok = gettok(p);
                if (tok->kind != ',') {
                    ungettok(p);
                    break;
                }
            }

            expect_or_error(p, ')', "missing ')' after function call");
            tree = new_node(NOD_CALL, tree, NULL);
            tree->sval = ident;

            return tree;
        } else {
            ungettok(p);

            tree = new_node(NOD_VAR, NULL, NULL);
            tree->sval = ident;

            /* array */
            tok = gettok(p);
            if (tok->kind == '[') {

                tree = new_node(NOD_ADD, tree, expression(p));

                expect_or_error(p, ']', "missing ']' at end of array");

                tree = new_node(NOD_DEREF, tree, NULL);

            } else {
                ungettok(p);
            }
        }
        return tree;
#endif

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
 *     | assignment_expression
 *     ;
 */
static struct ast_node *argument_expression(struct parser *p)
{
    struct ast_node *tree = NULL;

    tree = new_node(NOD_ARG, NULL, NULL);
    tree->l = assignment_expression(p);

    return tree;
}

/*
 * argument_expression_list
 *     : argument_expression_list
 *     | argument_expression_list ',' argument_expression
 *     ;
 */
static struct ast_node *argument_expression_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *expr = argument_expression(p);

        if (!expr)
            return tree;

        tree = new_node(NOD_LIST, tree, expr);

        if (!consume(p, ','))
            return tree;
    }
}

/*
 * postfix_expression
 *     : primary_expression
 *     | postfix_expression '.' TOK_IDENT
 *     ;
 */
static struct ast_node *postfix_expression(struct parser *p)
{
    struct ast_node *tree = primary_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '.':
            /* TODO fix this identifier */
            tree = new_node(NOD_STRUCT_REF, tree, identifier(p));
            return tree;

        case '(':
            tree = new_node(NOD_CALL, tree, NULL);
            tree->r = argument_expression_list(p);
            expect(p, ')');
            return tree;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * unary_expression
 *     : '+' primary_expression
 *     | '-' primary_expression
 *     | '*' unary_expression
 *     | '&' unary_expression
 *     ;
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
 *     : unary_expression
 *     | multiplicative_expression '*' unary_expression
 *     | multiplicative_expression '/' unary_expression
 *     ;
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
 *     : multiplicative_expression
 *     | additive_expression '+' multiplicative_expression
 *     | additive_expression '-' multiplicative_expression
 *     ;
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
 *     : additive_expression
 *     | relational_expression '<' additive_expression
 *     | relational_expression '>' additive_expression
 *     | relational_expression TOK_LE additive_expression
 *     | relational_expression TOK_GE additive_expression
 *     ;
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
 *     : relational_expression
 *     | equality_expression TOK_EQ relational_expression
 *     | equality_expression TOK_NE relational_expression
 *     ;
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
 *     : equality_expression
 *     | primary_expression '=' assignment_expression
 *     ;
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
 *     : assignment_expression
 *     ;
 */
static struct ast_node *expression(struct parser *p)
{
    return assignment_expression(p);
}

/*
 * compound_statement
 *     : statement
 *     | compound_statement statement
 *     ;
 */
static struct ast_node *compound_statement(struct parser *p)
{
    struct ast_node *tree = NULL;

    expect_or_error(p, '{', "missing '{'");

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '}':
            return new_node(NOD_COMPOUND, tree, NULL);

        case TOK_EOF:
            error(p, "missing '}' at end of file");
            return tree;

        default:
            ungettok(p);
            tree = new_node(NOD_STMT, tree, statement(p));
            break;
        }
    }
}

static struct ast_node *var_def(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *type = NULL;
    const struct token *tok = NULL;
    const char *ident = NULL;

    /* type */
    tok = gettok(p);
    switch (tok->kind) {
    case TOK_CHAR:
        type = new_node(NOD_TYPE_CHAR, NULL, NULL);
        break;
    case TOK_INT:
        type = new_node(NOD_TYPE_INT, NULL, NULL);
        break;
    case TOK_STRUCT:
        /* tag */
        tok = gettok(p);
        type = new_node(NOD_TYPE_STRUCT, NULL, NULL);
        type->sval = tok->text;
        break;
    default:
        error(p, "missing type name in declaration");
        break;
    }

    /* pointer */
    tok = gettok(p);
    if (tok->kind == '*') {
        type = new_node(NOD_TYPE_POINTER, type, NULL);
    } else {
        ungettok(p);
    }

    /* identifier */
    tok = gettok(p);
    if (tok->kind != TOK_IDENT) {
        error(p, "missing variable name");
        return NULL;;
    }
    ident = tok->text;

    /* array */
    tok = gettok(p);
    if (tok->kind == '[') {
        int array_len = 0;

        tok = gettok(p);
        if (tok->kind != TOK_NUM) {
            error(p, "missing constant after array '['");
        }
        array_len = tok->value;

        expect_or_error(p, ']', "missing ']' at end of array definition");

        type = new_node(NOD_TYPE_ARRAY, type, NULL);
        /* TODO find the best place to convert num */
        type->data.ival = array_len;
    } else {
        ungettok(p);
    }

    /* commit */
    tree = new_node(NOD_VAR_DEF, NULL, NULL);

    /* initialization */
    tok = gettok(p);
    if (tok->kind == '=') {
        tree->r = expression(p);
    } else {
        ungettok(p);
    }

    tree->l = type;
    tree->sval = ident;

    expect_or_error(p, ';', "missing ';' at end of declaration");

    return tree;
}

/*
 * statement
 *     : expression ';'
 *     | TOK_RETURN expression ';'
 *     ;
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
        tree->r = new_node(NOD_EXT, statement(p), NULL);
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

    case TOK_CHAR:
    case TOK_INT:
    case TOK_STRUCT:
        ungettok(p);
        return var_def(p);

    default:
        ungettok(p);
        tree = expression(p);
        expect_or_error(p, ';', "missing ';' at end of statement");
        break;
    }

    return tree;
}

static struct ast_node *func_params(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = NULL;
    int i;

    expect_or_error(p, '(', "missing '(' after function name");

    /* TODO remove limit 6 params */
    for (i = 0; i < 6; i++) {
        struct ast_node *param = NULL;
        struct ast_node *type = NULL;
        const char *ident = NULL;

        tok = gettok(p);
        if (tok->kind != TOK_INT) {
            ungettok(p);
            break;
        }
        /* TODO support more types */
        type = new_node(NOD_TYPE_INT, NULL, NULL);

        tok = gettok(p);
        if (tok->kind != TOK_IDENT) {
            error(p, "missing parameter name");
            break;
        }
        ident = tok->text;

        param = new_node(NOD_PARAM_DEF, NULL, NULL);
        param->l = type;
        param->sval = ident;

        tree = new_node(NOD_LIST, tree, param);

        tok = gettok(p);
        if (tok->kind != ',') {
            ungettok(p);
            break;
        }
    }

    expect_or_error(p, ')', "missing ')' after function params");

    return tree;
}

static struct ast_node *global_entry(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *type = NULL;
    const struct token *tok = NULL;
    const char *ident = NULL;

    /* type */
    tok = gettok(p);
    switch (tok->kind) {
    case TOK_CHAR:
        type = new_node(NOD_TYPE_CHAR, NULL, NULL);
        break;
    case TOK_INT: 
        type = new_node(NOD_TYPE_INT, NULL, NULL);
        break;
    default:
        error(p, "missing type name in declaration");
        break;
    }

    /* identifier */
    tok = gettok(p);
    if (tok->kind != TOK_IDENT) {
        error(p, "missing identifier");
    }
    ident = tok->text;

    /* func or var */
    tok = gettok(p);
    if (tok->kind == '(') {
        ungettok(p);

        tree = new_node(NOD_FUNC_BODY, NULL, NULL);

        tree->l = func_params(p);
        tree->r = compound_statement(p);
        /* TODO check if this is necessary */
        tree->sval = ident;

        tree = new_node(NOD_FUNC_DEF, NULL, tree);
        tree->l = type;
        tree->sval = ident;
    } else {
        ungettok(p);

        tree = new_node(NOD_VAR_DEF, NULL, NULL);

        /* initialization */
        tok = gettok(p);
        if (tok->kind == '=') {
            tree->r = expression(p);
        } else {
            ungettok(p);
        }

        /* TODO move these right after new node */
        tree->l = type;
        tree->sval = ident;

        expect_or_error(p, ';', "missing ';' at end of statement");
    }

    return tree;
}

static struct ast_node *struct_decl(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *members = NULL;
    const struct token *tok = NULL;

    expect_or_error(p, TOK_STRUCT, "missing struct");

    /* identifier */
    tok = gettok(p);
    if (tok->kind != TOK_IDENT) {
        error(p, "missing identifier");
    }

    tree = new_node(NOD_STRUCT_DECL, NULL, NULL);

    tree->sval = tok->text;

    expect_or_error(p, '{', "missing '{' after struct tag");

    for (;;) {
        tok = gettok(p);
        switch (tok->kind) {
        case TOK_CHAR:
        case TOK_INT:
            ungettok(p);
            members = new_node(NOD_MEMBER_DECL, members, var_def(p));
            break;

        default:
            ungettok(p);
            goto end_member;
        }
    }

end_member:
    tree->l = members;

    expect_or_error(p, '}', "missing '}' after struct declaration");
    expect_or_error(p, ';', "missing ';' at end of struct declaration");

    return tree;
}

/* XXX */
#define NEW_(kind) new_node(kind, NULL, NULL)
typedef struct parser Parser;
typedef struct ast_node Node;
typedef struct token Token;
static struct ast_node *translation_unit(struct parser *p);

struct ast_node *parse(struct parser *p)
{
    /* XXX */
    if (0) {

    struct ast_node *tree = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_CHAR:
        case TOK_INT:
            ungettok(p);
            tree = new_node(NOD_GLOBAL, tree, global_entry(p));
            break;

        case TOK_STRUCT:
            ungettok(p);
            tree = new_node(NOD_GLOBAL, tree, struct_decl(p));
            break;

        case TOK_EOF:
            return tree;

        default:
            error(p, "unexpected token in global scope");
            return tree;
        }
    }

    /* XXX */
    } else {
        return translation_unit(p);
    }
}

static struct ast_node *type_spec(struct parser *p);
static struct ast_node *decl_spec(struct parser *p);
static struct ast_node *declarator(struct parser *p);

static struct ast_node *ident2(struct parser *p)
{
    struct ast_node *tree = NULL;
    const Token *tok = consume(p, TOK_IDENT);

    if (!tok)
        return tree;

    tree = NEW_(NOD_DECL_IDENT);
    tree->sval = tok->text;

    return tree;
}

static struct ast_node *struct_decl2(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *spec = NULL;

    spec = type_spec(p);
    if (!spec)
        return NULL;

    tree = NEW_(NOD_MEMBER_DECL);
    tree->r = spec;
    tree->l = declarator(p);

    return tree;
}

static struct ast_node *struct_decl_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *member = struct_decl2(p);

        if (!member)
            return tree;

        tree = new_node(NOD_LIST, tree, member);
        expect(p, ';');
    }
}

static struct ast_node *struct_union(struct parser *p)
{
    struct ast_node *tree = NULL;
    const Token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_STRUCT:
        tree = NEW_(NOD_STRUCT_DECL);
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
    tree->l = ident2(p);

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
    const Token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_CHAR:
        tree = NEW_(NOD_TYPE_CHAR);
        break;

    case TOK_INT:
        tree = NEW_(NOD_TYPE_INT);
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
    tree->r = spec;
    tree->l = declarator(p);

    return tree;
}

static struct ast_node *param_list(struct parser *p)
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

    tree = NEW_(NOD_DIRECT_DECL);

    if (consume(p, '(')) {
        tree->l = declarator(p);
        expect(p, ')');
    }

    if (consume(p, TOK_IDENT)) {
        struct ast_node *ident = NEW_(NOD_DECL_IDENT);
        copy_token_text(p, ident);
        tree->l = ident;
    }

    if (consume(p, '[')) {
        struct ast_node *array = NEW_(NOD_TYPE_ARRAY);

        if (consume(p, TOK_NUM))
            copy_token_ival(p, array);

        tree->r = array;
        expect(p, ']');
    }

    if (consume(p, '(')) {
        struct ast_node *fn = new_node(NOD_DECL_FUNC, param_list(p), NULL);
        tree->r = fn;
        expect(p, ')');
    }

    return tree;
}

static struct ast_node *pointer(struct parser *p)
{
    struct ast_node *tree = NULL;

    while (consume(p, '*'))
        tree = new_node(NOD_TYPE_POINTER, tree, NULL);

    return tree;
}

static struct ast_node *declarator(struct parser *p)
{
    struct ast_node *tree = NEW_(NOD_DECLARATOR);

    tree->r = pointer(p);
    tree->l = direct_declarator(p);

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
 *     init_declarator_list
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
    struct ast_node *tree = NEW_(NOD_DECL);

    tree->r = decl_spec(p);
    tree->l = init_declarator_list(p);

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

static struct ast_node *extern_decl(struct parser *p)
{
    return declaration(p);
}

static struct ast_node *translation_unit(struct parser *p)
{
    struct ast_node *tree = NULL;

    while (!consume(p, TOK_EOF)) {
        /*
        struct ast_node *list = NEW_(NOD_LIST);

        list->l = tree;
        list->r = extern_decl(p);
        tree = list;
        */
        tree = new_node(NOD_LIST, tree, extern_decl(p));
    }

    return tree;
}
