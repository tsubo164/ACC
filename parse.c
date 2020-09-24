#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "type.h"

static struct symbol *lookup_symbol_(
            struct parser *p,
            const char *name, enum symbol_kind kind)
{
    return lookup_symbol(&p->symtbl, name, kind);
}

/* XXX
static struct symbol *insert_symbol_(
            struct parser *p,
            const char *name, enum symbol_kind kind)
{
    return insert_symbol(&p->symtbl, name, kind);
}
*/

static int scope_begin(struct parser *p)
{
    return symbol_scope_begin(&p->symtbl);
}

static int scope_end(struct parser *p)
{
    return symbol_scope_end(&p->symtbl);
}

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
 * forward declaration
 */
static struct ast_node *expression(struct parser *p);
static struct ast_node *statement(struct parser *p);

/*
 * primary_expression
 *     : TOK_NUM
 *     | TOK_IDENT
 *     | '(' expression ')'
 *     ;
 */
static struct ast_node *primary_expression(struct parser *p)
{
    struct ast_node *base = NULL;
    static char ident[TOKEN_WORD_SIZE] = {'\0'};
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_NUM:
        /* XXX base = new_number(tok->value); */
        base = new_node(NOD_NUM, NULL, NULL);
        base->data.ival = tok->value;
        base->dtype = type_int();
        return base;

    case TOK_IDENT:
        strcpy(ident, tok->word);
        tok = gettok(p);
        if (tok->kind == '(') {
            const struct symbol *sym;

            sym = lookup_symbol_(p, ident, SYM_FUNC);
            if (sym == NULL) {
                error(p, "calling undefined function");
                return NULL;
            }

            for (;;) {
                struct ast_node *expr = expression(p);
                if (expr == NULL) {
                    break;
                }

                base = new_node(NOD_ARG, base, expr);
                tok = gettok(p);
                if (tok->kind != ',') {
                    ungettok(p);
                    break;
                }
            }

            expect_or_error(p, ')', "missing ')' after function call");
            base = new_node(NOD_CALL, base, NULL);
            /*
            base->data.sym = sym;
            base->dtype = sym->dtype;
            */
            ast_node_set_symbol(base, sym);
            return base;
        } else {
            const struct symbol *sym;
            ungettok(p);

            sym = lookup_symbol_(p, ident, SYM_VAR);
            if (sym == NULL) {
                error(p, "using undeclared identifier");
                return NULL;
            }

            base = new_node(NOD_VAR, NULL, NULL);
            if (is_param(sym)) {
                base->kind = NOD_PARAM;
            } else if (is_global_var(sym)) {
                base->kind = NOD_GLOBAL_VAR;
            }
#if 0
            base->data.sym = sym;
            base->dtype = sym->dtype;
#endif
            ast_node_set_symbol(base, sym);

#if 1
            /* XXX ----------------------- */
            {
                /* array */
                tok = gettok(p);
                if (tok->kind == '[') {
                    /*
                    printf("    dtype: %s\n", data_type_to_string(base->dtype));
                    */

                    base = new_node(NOD_ADD, base, expression(p));
            /* XXX */
                    /*
            if (base->l->dtype->kind == DATA_TYPE_ARRAY) {
                struct ast_node *size, *mul;

                size = new_node(NOD_NUM, NULL, NULL);
                size->data.ival = base->l->dtype->ptr_to->byte_size;
                mul = new_node(NOD_MUL, size, base->r);
                base->r = mul;
            }
                    */

                    expect_or_error(p, ']', "missing ']' at end of array");

                    base = new_node(NOD_DEREF, base, NULL);
                    /* XXX */
                    /*
                    base->dtype = base->dtype->ptr_to;;
                    */
                    /*
                    printf("    dtype: %s\n", data_type_to_string(base->dtype));
                    */

                } else {
                    ungettok(p);
                }
            }
            /* XXX ----------------------- */
#endif
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
 *     | '*' unary_expression
 *     | '&' unary_expression
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

    case '*':
        base = new_node(NOD_DEREF, unary_expression(p), NULL);
        /* XXX */
        /*
        base->dtype = base->dtype->ptr_to;;
        */
        return base;

    case '&':
        base = new_node(NOD_ADDR, unary_expression(p), NULL);
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
            /* XXX */
            /*
            if (base->l->dtype->kind == DATA_TYPE_ARRAY) {
                struct ast_node *size, *mul;

                size = new_node(NOD_NUM, NULL, NULL);
                size->data.ival = base->l->dtype->ptr_to->byte_size;
                mul = new_node(NOD_MUL, size, base->r);
                base->r = mul;
            }
            */
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
 *     | relational_expression TOK_LE additive_expression
 *     | relational_expression TOK_GE additive_expression
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

        case TOK_LE:
            base = new_node(NOD_LE, base, additive_expression(p));
            break;

        case TOK_GE:
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
 *     | equality_expression TOK_EQ relational_expression
 *     | equality_expression TOK_NE relational_expression
 *     ;
 */
static struct ast_node *equality_expression(struct parser *p)
{
    struct ast_node *base = relational_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_EQ:
            base = new_node(NOD_EQ, base, relational_expression(p));
            break;

        case TOK_NE:
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

    scope_begin(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '}':
            goto final;

        case TOK_EOF:
            error(p, "missing '}' at end of file");
            goto final;

        default:
            ungettok(p);
            tree = new_node(NOD_STMT, tree, statement(p));
            break;
        }
    }

final:
    scope_end(p);
    return tree;
}

static struct ast_node *var_def(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct symbol *sym = NULL;
    /* XXX this would change to non const */
    struct data_type *dtype = NULL;
    const struct token *tok = NULL;

#if 0
    /* type */
    tok = gettok(p);
    if (tok->kind != TOK_INT) {
        error(p, "missing type name in declaration");
    }
    dtype = type_int();
#endif
    /* type */
    tok = gettok(p);
    switch (tok->kind) {
    case TOK_CHAR:
        dtype = type_char();
        break;
    case TOK_INT: 
        dtype = type_int();
        break;
    default:
        error(p, "missing type name in declaration");
        break;
    }

    /* pointer */
    tok = gettok(p);
    if (tok->kind == '*') {
        struct data_type *curr_type = dtype;
        dtype = type_ptr();
        dtype->ptr_to = curr_type;
    } else {
        ungettok(p);
    }

    /* identifier */
    tok = gettok(p);
    if (tok->kind != TOK_IDENT) {
        error(p, "missing variable name");
        return NULL;;
    }

#if 0
    sym = insert_symbol_(p, tok->word, SYM_VAR);
    if (sym == NULL) {
        error(p, "redefinition of variable");
        return NULL;
    }
#endif
    /* XXX */
    sym = define_variable(&p->symtbl, tok->word);
    /* XXX */
    sym->file_pos = tok->file_pos;

    /* array */
    tok = gettok(p);
    if (tok->kind == '[') {
        struct data_type *curr_type = dtype;
        int array_len = 0;

        tok = gettok(p);
        if (tok->kind != TOK_NUM) {
            error(p, "missing constant after array '['");
        }
        array_len = tok->value;

        expect_or_error(p, ']', "missing ']' at end of array definition");

        dtype = type_array(curr_type, array_len);
    } else {
        ungettok(p);
    }

    /* commit var */

    tree = new_node(NOD_VAR_DEF, NULL, NULL);
#if 0
    sym->dtype = dtype;
    tree->data.sym = sym;
#endif
    sym->dtype = dtype;
    ast_node_set_symbol(tree, sym);

    /* initialization */
    tok = gettok(p);
    if (tok->kind == '=') {
        tree->l = expression(p);
    } else {
        ungettok(p);
    }

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
    struct ast_node *base = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_RETURN:
        base = new_node(NOD_RETURN, expression(p), NULL);
        expect_or_error(p, ';', "missing ';' at end of return statement");
        break;

    case TOK_IF:
        expect_or_error(p, '(', "missing '(' after if");
        base = new_node(NOD_IF, expression(p), NULL);
        expect_or_error(p, ')', "missing ')' after if condition");
        base->r = new_node(NOD_EXT, statement(p), NULL);
        tok = gettok(p);
        if (tok->kind == TOK_ELSE) {
            base->r->r = statement(p);
        } else {
            ungettok(p);
        }
        break;

    case TOK_WHILE:
        expect_or_error(p, '(', "missing '(' after while");
        base = new_node(NOD_WHILE, expression(p), NULL);
        expect_or_error(p, ')', "missing ')' after while condition");
        base->r = statement(p);
        break;

    case '{':
        ungettok(p);
        return compound_statement(p);

    case TOK_CHAR:
    case TOK_INT:
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

static struct ast_node *func_params(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = NULL;
    int nparams = 0;
    int i;

    expect_or_error(p, '(', "missing '(' after function name");

    /* XXX limit 6 params */
    for (i = 0; i < 6; i++) {
        struct symbol *sym = NULL;

        tok = gettok(p);
        if (tok->kind != TOK_INT) {
            ungettok(p);
            break;
        }

        tok = gettok(p);
        if (tok->kind != TOK_IDENT) {
            error(p, "missing parameter name");
            break;
        }

        tree = new_node(NOD_PARAM, tree, NULL);

        /* XXX */
        sym = define_variable(&p->symtbl, tok->word);
        sym->kind = SYM_PARAM;
        sym->dtype = type_int();

        tree->data.sym = sym;
        tree->dtype = sym->dtype;
        nparams++;

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
    struct symbol *sym = NULL;
    const struct token *tok = NULL;
    const struct data_type *dtype = NULL;
    static char ident[TOKEN_WORD_SIZE] = {'\0'};
    /* XXX */
    long fpos;

#if 0
    tok = gettok(p);
    if (tok->kind != TOK_INT) {
        error(p, "missing type before ideintifier");
    }
    dtype = type_int();
#endif
    /* type */
    tok = gettok(p);
    switch (tok->kind) {
    case TOK_CHAR:
        dtype = type_char();
        break;
    case TOK_INT: 
        dtype = type_int();
        break;
    default:
        error(p, "missing type name in declaration");
        break;
    }

    /* ideintifier */
    tok = gettok(p);
    if (tok->kind != TOK_IDENT) {
        error(p, "missing ideintifier");
    }
    strcpy(ident, tok->word);
    /* XXX */
    fpos = tok->file_pos;

    /* func or var */
    tok = gettok(p);
    if (tok->kind == '(') {
        ungettok(p);

        /* XXX */
        sym = define_function(&p->symtbl, ident);

        tree = new_node(NOD_FUNC_DEF, NULL, NULL);

        sym->dtype = dtype;
        ast_node_set_symbol(tree, sym);

        scope_begin(p);

        tree->l = func_params(p);
        tree->r = compound_statement(p);

        scope_end(p);

    } else {
        ungettok(p);

        /* XXX */
        sym = define_variable(&p->symtbl, ident);

        tree = new_node(NOD_VAR_DEF, NULL, NULL);
        sym->dtype = dtype;
        ast_node_set_symbol(tree, sym);

        /* XXX initialization */
        tok = gettok(p);
        if (tok->kind == '=') {
            tree->l = expression(p);
            if (tree->l->kind == NOD_NUM) {
                sym->mem_offset = tree->l->data.ival;
            }
        } else {
            ungettok(p);
        }

        /* XXX */
        sym->file_pos = fpos;
        expect_or_error(p, ';', "missing ';' at end of statement");
    }

    return tree;
}

struct ast_node *parse(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_CHAR:
        case TOK_INT:
            ungettok(p);
            tree = new_node(NOD_GLOBAL, tree, global_entry(p));
            break;

        case TOK_EOF:
            return tree;

        default:
            error(p, "unexpected token in global scope");
            return tree;
        }
    }
}
