#include <stdlib.h>
#include <string.h>
#include "parse.h"

static struct symbol symtbl[32] = {{NULL, 0}};
static int nsyms = 0;
static int nvars = 0;

static const struct symbol *lookup_symbol2(const char *name, enum symbol_kind kind)
{
    struct symbol *sym = NULL;
    int i;

    for (i = 0; i < nsyms; i++) {
        sym = &symtbl[i];

        if (!strcmp(sym->name, name) /*&& sym->kind == kind*/) {
            return sym;
        }
    }

    return NULL;
}

static const struct symbol *insert_symbol2(const char *name, enum symbol_kind kind)
{
    struct symbol *sym = NULL;

    if (lookup_symbol2(name, kind) != NULL) {
        return NULL;
    }

    sym = &symtbl[nsyms++];

    sym->name = malloc(strlen(name) + 1);
    strcpy(sym->name, name);
    sym->kind = kind;
    sym->offset = 0;

    if (kind == SYM_VAR || kind == SYM_PARAM) {
        nvars++;
        sym->offset = 8 * nvars;
    }

    return sym;
}

static struct ast_node *new_node(enum ast_node_kind kind,
        struct ast_node *l, struct ast_node *r)
{
    struct ast_node *n = malloc(sizeof(struct ast_node));
    n->kind = kind;
    n->l = l;
    n->r = r;
    n->data.ival = 0;

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
static struct ast_node *statement(struct parser *p);

/*
 * primary_expression
 *     : TK_NUM
 *     | TK_IDENT
 *     | '(' expression ')'
 *     ;
 */
static struct ast_node *primary_expression(struct parser *p)
{
    struct ast_node *base = NULL;
    static char ident[TOKEN_WORD_SIZE] = {'\0'};
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TK_NUM:
        /* XXX base = new_number(tok->value); */
        base = new_node(NOD_NUM, NULL, NULL);
        base->data.ival = tok->value;
        return base;

    case TK_IDENT:
        strcpy(ident, tok->word);
        tok = gettok(p);
        if (tok->kind == '(') {
            const struct symbol *sym;

            sym = lookup_symbol2(ident, SYM_FUNC);
            if (sym == NULL) {
                error(p, "calling undefined function");
                return NULL;
            }

            for (;;) {
                base = new_node(NOD_ARG, base, expression(p));
                tok = gettok(p);
                if (tok->kind != ',') {
                    ungettok(p);
                    break;
                }
            }

            expect_or_error(p, ')', "missing ')' after function call");
            base = new_node(NOD_CALL, base, NULL);
            base->data.sym = sym;
            return base;
        } else {
            const struct symbol *sym;
            ungettok(p);

            sym = lookup_symbol2(ident, SYM_VAR);
            if (sym == NULL) {
                error(p, "using undeclared identifier");
                return NULL;
            }

            base = new_node(NOD_VAR, NULL, NULL);
            if (sym->kind == SYM_PARAM) {
                base->kind = NOD_PARAM;
            }
            base->data.sym = sym;
        }
        return base;

    case '(':
        base = expression(p);
        expect(p, ')');
        return base;

    default:
        /* XXX
         * when parser expects an expression, does it accept
         * blank or treat as an error?
         */
        ungettok(p);
        return NULL;
        /*
        error(p, "unexpected token");
        return NULL;
        */
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
    struct ast_node *base = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case '+':
        base = primary_expression(p);
        return base;

    case '-':
        /* ??? base = new_number(-1); */
        base = new_node(NOD_NUM, NULL, NULL);
        base->data.ival = -1;
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
            return tree;

        case TK_EOF:
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
    const struct symbol *sym = NULL;
    const struct token *tok = NULL;

    tok = gettok(p);
    if (tok->kind != TK_INT) {
        error(p, "missing type name in declaration");
    }

    tok = gettok(p);
    if (tok->kind != TK_IDENT) {
        error(p, "missing parameter name");
        return tree;;
    }

    sym = insert_symbol2(tok->word, SYM_VAR);
    if (sym == NULL) {
        error(p, "redefinition of variable");
        return NULL;
    }

    tree = new_node(NOD_VAR, NULL, NULL);
    tree->data.sym = sym;

    expect_or_error(p, ';', "missing ';' at end of declaration");

    return tree;
}

/*
 * statement
 *     : expression ';'
 *     | TK_RETURN expression ';'
 *     ;
 */
static struct ast_node *statement(struct parser *p)
{
    struct ast_node *base = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TK_RETURN:
        base = new_node(NOD_RETURN, expression(p), NULL);
        expect_or_error(p, ';', "missing ';' at end of return statement");
        break;

    case TK_IF:
        expect_or_error(p, '(', "missing '(' after if");
        base = new_node(NOD_IF, expression(p), NULL);
        expect_or_error(p, ')', "missing ')' after if condition");
        base->r = new_node(NOD_EXT, statement(p), NULL);
        tok = gettok(p);
        if (tok->kind == TK_ELSE) {
            base->r->r = statement(p);
        } else {
            ungettok(p);
        }
        break;

    case TK_WHILE:
        expect_or_error(p, '(', "missing '(' after while");
        base = new_node(NOD_WHILE, expression(p), NULL);
        expect_or_error(p, ')', "missing ')' after while condition");
        base->r = statement(p);
        break;

    case '{':
        ungettok(p);
        return compound_statement(p);

    case TK_INT:
        ungettok(p);
        return var_def(p);

    default:
        ungettok(p);
        base = expression(p);
        expect_or_error(p, ';', "missing ';' at end of statement");
        break;
    }

    return base;
}

static struct ast_node *func_def(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *params = NULL;
    const struct symbol *sym = NULL;
    const struct token *tok = NULL;
    int nparams = 0;
    int i;

    nvars = 0;

    tok = gettok(p);
    if (tok->kind != TK_INT) {
        error(p, "missing return type after function name");
    }

    tok = gettok(p);
    if (tok->kind != TK_IDENT) {
        error(p, "missing function name");
    }

    tree = new_node(NOD_FUNC_DEF, NULL, NULL);

    sym = insert_symbol2(tok->word, SYM_FUNC);
    if (sym == NULL) {
        error(p, "redefinition of function");
        return NULL;
    }

    tree->data.sym = sym;
    /*
    printf("### %s\n", tree->data.sym->name);
    */

    expect_or_error(p, '(', "missing '(' after function name");

    /*
    for (;;) {
    */
    for (i = 0; i < 5; i++) {
        const struct symbol *symparam = NULL;

        tok = gettok(p);
        if (tok->kind != TK_INT) {
            ungettok(p);
            break;
        }

        tok = gettok(p);
        if (tok->kind != TK_IDENT) {
            error(p, "missing parameter name");
            break;
        }

        params = new_node(NOD_PARAM, params, NULL);

        symparam = insert_symbol2(tok->word, SYM_PARAM);
        if (symparam == NULL) {
            error(p, "redefinition of parameter");
            return NULL;
        }

        params->data.sym = symparam;
        nparams++;
        /*
        printf("        - %s\n", tok->word);
        */

        tok = gettok(p);
        if (tok->kind != ',') {
            ungettok(p);
            break;
        }
    }
        /*
        printf("nparams(%s) -> %d\n", tree->data.sym->name, nparams);
        */

    tree->l = params;

    expect_or_error(p, ')', "missing ')' after function params");

    tree->r = compound_statement(p);
    return tree;
}

struct ast_node *parse(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TK_INT:
            ungettok(p);
            tree = new_node(NOD_LIST, tree, func_def(p));
            break;

        case TK_EOF:
            return tree;

        default:
            error(p, "unexpected token in global scope");
            return tree;
        }
    }
}
