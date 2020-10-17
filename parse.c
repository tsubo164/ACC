#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "type.h"

/* TODO remove scope functions later */
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
 * forward declaration
 */
static struct ast_node *expression(struct parser *p);
static struct ast_node *statement(struct parser *p);

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
 * primary_expression
 *     : TOK_NUM
 *     | TOK_IDENT
 *     | '(' expression ')'
 *     ;
 */
static struct ast_node *primary_expression(struct parser *p)
{
    struct ast_node *tree = NULL;
    const char *ident = NULL;
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
        ident = tok->text;

        tok = gettok(p);
        if (tok->kind == '(') {
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
    tree = new_node(NOD_COMPOUND, tree, NULL);
    scope_end(p);

    return tree;
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
    int nparams = 0;
    int i;

    expect_or_error(p, '(', "missing '(' after function name");

    /* XXX limit 6 params */
    for (i = 0; i < 6; i++) {
        struct symbol *sym = NULL;

        struct ast_node *param = NULL;
        struct ast_node *type = NULL;
        const char *ident = NULL;
        tok = gettok(p);
        if (tok->kind != TOK_INT) {
            ungettok(p);
            break;
        }
        type = new_node(NOD_TYPE_INT, NULL, NULL);

        tok = gettok(p);
        if (tok->kind != TOK_IDENT) {
            error(p, "missing parameter name");
            break;
        }
        ident = tok->text;

#if 0
        tree = new_node(NOD_PARAM_DEF, NULL, tree);

        /* XXX */
        /*
        sym = define_variable(&p->symtbl, tok->word);
        */
        sym = define_variable(&p->symtbl, tok->text);
        sym->kind = SYM_PARAM;
        sym->dtype = type_int();

        tree->data.sym = sym;
        tree->dtype = sym->dtype;
        nparams++;

        /* TODO change to ->l */
        tree->l = type;
        tree->sval = ident;
#else
        param = new_node(NOD_PARAM_DEF, NULL, NULL);

        sym = define_variable(&p->symtbl, tok->text);
        sym->kind = SYM_PARAM;
        sym->dtype = type_int();

        param->data.sym = sym;
        param->dtype = sym->dtype;
        nparams++;

        param->l = type;
        param->sval = ident;

        tree = new_node(NOD_LIST, tree, param);
#endif

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
    /* XXX */
    long fpos;

    struct ast_node *type = NULL;
    const char *ident = NULL;
#if 0
    tok = gettok(p);
    if (tok->kind != TOK_INT) {
        error(p, "missing type before identifier");
    }
    dtype = type_int();
#endif
    /* type */
    tok = gettok(p);
    switch (tok->kind) {
    case TOK_CHAR:
        dtype = type_char();
        type = new_node(NOD_TYPE_CHAR, NULL, NULL);
        break;
    case TOK_INT: 
        dtype = type_int();
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
    /* XXX */
    fpos = tok->file_pos;

    ident = tok->text;

    /* func or var */
    tok = gettok(p);
    if (tok->kind == '(') {
        ungettok(p);

        /* XXX */
        sym = define_function(&p->symtbl, ident);

        tree = new_node(NOD_FUNC_BODY, NULL, NULL);

        /*
        sym->dtype = dtype;
        ast_node_set_symbol(tree, sym);
        */

        scope_begin(p);

        tree->l = func_params(p);
        tree->r = compound_statement(p);
        tree->sval = ident;

        /* TODO scope */
        /*
        tree = new_node(NOD_SCOPE, tree, NULL);
        */

        tree = new_node(NOD_FUNC_DEF, NULL, tree);
        sym->dtype = dtype;
        ast_node_set_symbol(tree, sym);

        /* TODO change to ->l */
        tree->l = type;
        tree->sval = ident;
        /*
        */

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
            tree->r = expression(p);
            if (tree->r->kind == NOD_NUM) {
                sym->mem_offset = tree->r->data.ival;
            }
        } else {
            ungettok(p);
        }

        /* TODO change to ->l */
        tree->l = type;
        tree->sval = ident;
        /*
        */

        /* XXX */
        sym->file_pos = fpos;
        expect_or_error(p, ';', "missing ';' at end of statement");
    }

    return tree;
}

static struct ast_node *struct_decl(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *members = NULL;
    struct symbol *sym = NULL;

    /*
    */
    const struct token *tok = NULL;
    struct token ident;

    expect_or_error(p, TOK_STRUCT, "missing struct");

    /* identifier */
    tok = gettok(p);
    if (tok->kind != TOK_IDENT) {
        error(p, "missing identifier");
    }
    ident = *tok;

    tree = new_node(NOD_STRUCT_DECL, NULL, NULL);

    tree->sval = tok->text;

    /* XXX */
    /*
    sym = define_struct(&p->symtbl, ident.word);
    */
    sym = define_struct(&p->symtbl, ident.text);
    scope_begin(p);

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
    scope_end(p);

        /* XXX */
    /*
        sym->dtype = type_struct(sym->name);
        ast_node_set_symbol(tree, sym);
    */

    tree->l = members;

    expect_or_error(p, '}', "missing '}' after struct declaration");
    expect_or_error(p, ';', "missing ';' at end of struct declaration");

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
}
