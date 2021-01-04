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
        return &p->tokbuf[p->curr];
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
    node->ival = tok->value;
}

static const struct token *consume(struct parser *p, int token_kind)
{
    const struct token *tok = gettok(p);

    if (tok->kind == token_kind)
        return tok;

    ungettok(p);
    return NULL;
}

static int nexttok(struct parser *p, int token_kind)
{
    const struct token *tok = gettok(p);
    const int kind = tok->kind;

    ungettok(p);
    return kind == token_kind;
}

static int peektok(struct parser *p)
{
    const struct token *tok = gettok(p);
    const int kind = tok->kind;

    ungettok(p);
    return kind;
}

static void syntax_error(struct parser *p, const char *msg)
{
    if (!p->is_panic_mode) {
        const struct token *tok = current_token(p);
        add_error(p->msg, msg, &tok->pos);
        p->is_panic_mode = 1;
    }
}

static void expect(struct parser *p, enum token_kind query)
{
    const struct token *tok = gettok(p);

    if (p->is_panic_mode) {
        if (tok->kind == query || tok->kind == TOK_EOF) {
            p->is_panic_mode = 0;
        }
    } else {
        if (tok->kind == query)
            return;

        {
            char buf[1024] = {'\0'};
            const char *msg = NULL;
            /* TODO improve error message */
            sprintf(buf, "expected token '%c' here", query);
            msg = insert_string(p->lex.strtab, buf);
            syntax_error(p, msg);
        }
    }
}

struct parser *new_parser()
{
    struct parser *p = malloc(sizeof(struct parser));
    int i;

    for (i = 0; i < TOKEN_BUFFER_SIZE; i++)
        token_init(&p->tokbuf[i]);

    lexer_init(&p->lex);
    p->head = 0;
    p->curr = 0;

    p->error_pos = -1L;
    p->error_msg = "";

    p->decl_kind = 0;
    p->decl_ident = NULL;
    p->decl_type = NULL;
    p->is_panic_mode = 0;

    return p;
}

void free_parser(struct parser *p)
{
    free(p);
}

/* decl context */
static void define_sym(struct parser *p, struct ast_node *node)
{
    struct symbol *sym;

    sym = define_symbol(p->symtab, p->decl_ident, p->decl_kind, p->decl_type);
    node->sym = sym;
}

static void use_sym(struct parser *p, struct ast_node *node)
{
    struct symbol *sym;

    /* TODO support incomplete struct/union/enum
     * currently registered as an int type */
    sym = use_symbol(p->symtab, p->decl_ident, p->decl_kind);
    node->sym = sym;
}

static void define_case(struct parser *p, struct ast_node *node, int kind)
{
    struct symbol *sym;

    sym = define_case_symbol(p->symtab, kind);
    node->sym = sym;
}

static void define_label(struct parser *p, struct ast_node *node)
{
    struct symbol *sym;

    sym = define_symbol(p->symtab, p->decl_ident, SYM_LABEL, type_int());
    node->sym = sym;
}

static void scope_begin(struct parser *p)
{
    symbol_scope_begin(p->symtab);
}

static void scope_end(struct parser *p)
{
    symbol_scope_end(p->symtab);
}

static void switch_begin(struct parser *p)
{
    symbol_switch_begin(p->symtab);
}

static void switch_end(struct parser *p)
{
    symbol_switch_end(p->symtab);
}

static void decl_begin(struct parser *p, int decl_kind)
{
    p->decl_kind = decl_kind;
}

static void decl_set_ident(struct parser *p, const char *decl_ident)
{
    p->decl_ident = decl_ident;
}

static void decl_set_type(struct parser *p, struct data_type *decl_type)
{
    p->decl_type = decl_type;
}

static int decl_is_func(struct parser *p)
{
    return p->decl_kind == SYM_FUNC || p->decl_kind == SYM_PARAM;
}

/*
 * forward declarations
 */
static struct ast_node *statement(struct parser *p);
static struct ast_node *expression(struct parser *p);
static struct ast_node *assignment_expression(struct parser *p);
static struct ast_node *decl_identifier(struct parser *p);
static struct ast_node *type_specifier(struct parser *p);
static struct ast_node *declaration_specifier(struct parser *p);
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

    if (!consume(p, TOK_IDENT))
        return NULL;

    tree = new_node(NOD_IDENT, NULL, NULL);
    copy_token_text(p, tree);
    decl_set_ident(p, tree->sval);

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
        copy_token_text(p, tree);
        copy_token_ival(p, tree);
        return tree;

    case TOK_STRING_LITERAL:
        tree = new_node(NOD_STRING, NULL, NULL);
        copy_token_text(p, tree);
        copy_token_ival(p, tree);
        return tree;

    case TOK_IDENT:
        ungettok(p);
        tree = identifier(p);
        use_sym(p, tree);
        return tree;

    case '(':
        tree = expression(p);
        expect(p, ')');
        return tree;

    default:
        ungettok(p);
        syntax_error(p, "not an expression");
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
 *     postfix_expression '[' expression ']'
 *     postfix_expression '(' argument_expression_list ')'
 *     postfix_expression TOK_INC
 *     postfix_expression TOK_DEC
 */
static struct ast_node *postfix_expression(struct parser *p)
{
    struct ast_node *tree = primary_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '.':
            tree = new_node(NOD_STRUCT_REF, tree, identifier(p));
            /* TODO ADDSYM */
            {
                struct symbol *sym;
                const struct symbol *sym_l = tree->l->sym;
                const char *mem = tree->r->sval;
                const char *tag;

                tag = sym_l->type->tag;
                sym = lookup_symbol(p->symtab, tag, SYM_TAG_STRUCT);
                for (;;) {
                    /* TODO stop searching when struct is incomplete */
                    if (sym->kind == SYM_SCOPE_END) {
                        /* end of struct definition */
                        break;
                    }

                    if (sym->name && !strcmp(sym->name, mem)) {
                        tree->r->sym = sym;
                        /* TODO adding type to node? */
                        tree->r->type = sym->type;
                        tree->sym = sym;
                        tree->type = sym->type;
                        break;
                    }
                    sym = next(sym);
                }
            }
            break;

        case '(':
            tree = new_node(NOD_CALL, tree, NULL);
            if (!nexttok(p, ')'))
                tree->r = argument_expression_list(p);
            expect(p, ')');
            break;

        case '[':
            tree = new_node(NOD_ADD, tree, expression(p));
            expect(p, ']');
            tree = new_node(NOD_DEREF, tree, NULL);
            break;

        case TOK_INC:
            tree = new_node(NOD_POSTINC, tree, NULL);
            break;

        case TOK_DEC:
            tree = new_node(NOD_POSTDEC, tree, NULL);
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * unary_expression
 *     postfix_expression
 *     TOK_INC unary_expression
 *     TOK_DEC unary_expression
 *     unary_operator cast_expression
 *     TOK_SIZEOF unary_expression
 *     TOK_SIZEOF '(' TOK_TYPENAME ')'
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
        tree->ival = -1;
        tree = new_node(NOD_MUL, tree, postfix_expression(p));
        return tree;

    case '*':
        return new_node(NOD_DEREF, postfix_expression(p), NULL);

    case '&':
        return new_node(NOD_ADDR, postfix_expression(p), NULL);

    case '!':
        return new_node(NOD_NOT, postfix_expression(p), NULL);

    case TOK_INC:
        return new_node(NOD_PREINC, unary_expression(p), NULL);

    case TOK_DEC:
        return new_node(NOD_PREDEC, unary_expression(p), NULL);

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
 * inclusive_or_expression
 *     exclusive_or_expression
 *     inclusive_or_expression '|' exclusive_or_expression
 */
static struct ast_node *inclusive_or_expression(struct parser *p)
{
    return equality_expression(p);
}

/*
 * logical_and_expression
 *     inclusive_or_expression
 *     logical_and_expression TOK_LOGICAL_AND inclusive_or_expression
 */
static struct ast_node *logical_and_expression(struct parser *p)
{
    struct ast_node *tree = inclusive_or_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_LOGICAL_AND:
            tree = new_node(NOD_LOGICAL_AND, tree, inclusive_or_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * logical_or_expression
 *     logical_and_expression
 *     logical_or_expression TOK_LOGICAL_OR logical_and_expression
 */
static struct ast_node *logical_or_expression(struct parser *p)
{
    struct ast_node *tree = logical_and_expression(p);

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_LOGICAL_OR:
            tree = new_node(NOD_LOGICAL_OR, tree, logical_and_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * conditional_expression
 *     logical_or_expression
 *     logical_or_expression '?' expression ':' conditional_expression
 */
static struct ast_node *conditional_expression(struct parser *p)
{
    return logical_or_expression(p);
}

/*
 * assignment_expression
 *     conditional_expression
 *     unary_expression '=' assignment_expression
 *     unary_expression TOK_ADD_ASSIGN assignment_expression
 *     unary_expression TOK_SUB_ASSIGN assignment_expression
 *     unary_expression TOK_MUL_ASSIGN assignment_expression
 *     unary_expression TOK_DIV_ASSIGN assignment_expression
 */
static struct ast_node *assignment_expression(struct parser *p)
{
    struct ast_node *tree = conditional_expression(p);
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case '=':
        return new_node(NOD_ASSIGN, tree, assignment_expression(p));

    case TOK_ADD_ASSIGN:
        return new_node(NOD_ADD_ASSIGN, tree, assignment_expression(p));

    case TOK_SUB_ASSIGN:
        return new_node(NOD_SUB_ASSIGN, tree, assignment_expression(p));

    case TOK_MUL_ASSIGN:
        return new_node(NOD_MUL_ASSIGN, tree, assignment_expression(p));

    case TOK_DIV_ASSIGN:
        return new_node(NOD_DIV_ASSIGN, tree, assignment_expression(p));

    default:
        ungettok(p);
        return tree;
    }
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
 * constant_expression
 *     conditional_expression
 */
static struct ast_node *constant_expression(struct parser *p)
{
    return new_node(NOD_CONST_EXPR, conditional_expression(p), NULL);
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

    scope_begin(p);
    tree = new_node(NOD_COMPOUND, NULL, NULL);
    tree->l = declaration_list(p);
    tree->r = statement_list(p);
    scope_end(p);

    expect(p, '}');

    return tree;
}

/*
 * expression_statement
 *     ';'
 *     expression ';'
 */
static struct ast_node *expression_statement(struct parser *p)
{
    struct ast_node *tree = NULL;

    if (!nexttok(p, ';'))
        tree = expression(p);
    expect(p, ';');

    return tree;
}

/*
 * for_statement
 *     TOK_FOR '(' expression_statement expression_statement ')' statement
 *     TOK_FOR '(' expression_statement expression_statement expression ')' statement
 */
static struct ast_node *for_statement(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *pre = NULL, *cond = NULL, *body = NULL, *post = NULL;
    struct ast_node *pre_cond = NULL, *body_post = NULL;

    expect(p, TOK_FOR);
    expect(p, '(');
    pre  = expression_statement(p);
    cond = expression_statement(p);
    if (!nexttok(p, ')'))
        post = expression(p);
    expect(p, ')');
    body = statement(p);

    pre_cond  = new_node(NOD_FOR_PRE_COND, pre, cond);
    body_post = new_node(NOD_FOR_BODY_POST, body, post);
    tree      = new_node(NOD_FOR, pre_cond, body_post);

    return tree;
}

/*
 * while_statement
 *     TOK_WHILE '(' expression ')' statement
 */
static struct ast_node *while_statement(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *cond = NULL, *body = NULL;

    expect(p, TOK_WHILE);
    expect(p, '(');
    cond = expression(p);
    expect(p, ')');
    body = statement(p);

    tree = new_node(NOD_WHILE, cond, body);
    return tree;
}

/*
 * dowhile_statement
 *     TOK_DO statement TOK_WHILE '(' expression ')' ';'
 */
static struct ast_node *dowhile_statement(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *cond = NULL, *body = NULL;

    expect(p, TOK_DO);
    body = statement(p);
    expect(p, TOK_WHILE);

    expect(p, '(');
    cond = expression(p);
    expect(p, ')');
    expect(p, ';');

    tree = new_node(NOD_DOWHILE, body, cond);
    return tree;
}

/*
 * break_statement
 *     TOK_BREAK ';'
 */
static struct ast_node *break_statement(struct parser *p)
{
    struct ast_node *tree = NULL;

    expect(p, TOK_BREAK);
    expect(p, ';');

    tree = NEW_(NOD_BREAK);
    return tree;
}

/*
 * continue_statement
 *     TOK_CONTINUE ';'
 */
static struct ast_node *continue_statement(struct parser *p)
{
    struct ast_node *tree = NULL;

    expect(p, TOK_CONTINUE);
    expect(p, ';');

    tree = NEW_(NOD_CONTINUE);
    return tree;
}

/*
 * return_statement
 *     TOK_RETURN ';'
 *     TOK_RETURN expression ';'
 */
static struct ast_node *return_statement(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *expr = NULL;

    expect(p, TOK_RETURN);
    if (!nexttok(p, ';'))
        expr = expression(p);
    expect(p, ';');

    tree = new_node(NOD_RETURN, expr, NULL);
    return tree;
}

/*
 * goto_statement
 *     TOK_GOTO TOK_IDENT ';'
 */
static struct ast_node *goto_statement(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *ident = NULL;

    expect(p, TOK_GOTO);
    ident = identifier(p);
    expect(p, ';');

    tree = new_node(NOD_GOTO, ident, NULL);
    /*
    use_sym(p, tree->l);
    */
    return tree;
}

/*
 * if_statement
 *     TOK_IF '(' expression ')' statement
 *     TOK_IF '(' expression ')' statement TOK_ELSE statement
 */
static struct ast_node *if_statement(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *cond = NULL, *then = NULL, *els = NULL;
    struct ast_node *then_else = NULL;

    expect(p, TOK_IF);
    expect(p, '(');
    cond = expression(p);
    expect(p, ')');
    then = statement(p);
    if (consume(p, TOK_ELSE))
        els = statement(p);

    then_else = new_node(NOD_IF_THEN, then, els);
    tree      = new_node(NOD_IF, cond, then_else);

    return tree;
}

/*
 * switch_statement
 *     TOK_SWITCH '(' expression ')' statement
 */
static struct ast_node *switch_statement(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *expr = NULL, *body = NULL;

    expect(p, TOK_SWITCH);
    expect(p, '(');
    expr = expression(p);
    expect(p, ')');

    switch_begin(p);
    body = statement(p);
    switch_end(p);

    tree = new_node(NOD_SWITCH, expr, body);
    return tree;
}

/*
 * case_statement
 *     TOK_CASE constant_expression ':' statement
 */
static struct ast_node *case_statement(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *expr = NULL;

    expect(p, TOK_CASE);
    expr = constant_expression(p);
    expect(p, ':');

    tree = new_node(NOD_CASE, expr, NULL);
    define_case(p, tree, SYM_CASE);

    tree->r = statement(p);
    return tree;
}

/*
 * default_statement
 *     TOK_DEFAULT ':' statement
 */
static struct ast_node *default_statement(struct parser *p)
{
    struct ast_node *tree = NULL;

    expect(p, TOK_DEFAULT);
    expect(p, ':');

    tree = new_node(NOD_DEFAULT, NULL, NULL);
    define_case(p, tree, SYM_DEFAULT);

    tree->l = statement(p);
    return tree;
}

/*
 * labeled_statement
 *     TOK_IDENT ':' statement
 */
static struct ast_node *labeled_statement(struct parser *p)
{
    /* this statement is actuallyy for only goto statement
     * case and default statements are defined separately */
    struct ast_node *tree = NULL;
    struct ast_node *ident = NULL;

    ident = decl_identifier(p);
    expect(p, ':');

    tree = new_node(NOD_LABEL, ident, NULL);
    define_label(p, tree->l);

    tree->r = statement(p);
    return tree;
}

/*
 * statement
 *     labeled_statement
 *     compound_statement
 *     expression_statement
 *     selection_statement
 *     iteration_statement
 *     jump_statement
 */
static struct ast_node *statement(struct parser *p)
{
    switch (peektok(p)) {

    case TOK_FOR:
        return for_statement(p);

    case TOK_WHILE:
        return while_statement(p);

    case TOK_DO:
        return dowhile_statement(p);

    case TOK_BREAK:
        return break_statement(p);

    case TOK_CONTINUE:
        return continue_statement(p);

    case TOK_RETURN:
        return return_statement(p);

    case TOK_GOTO:
        return goto_statement(p);

    case TOK_IF:
        return if_statement(p);

    case TOK_SWITCH:
        return switch_statement(p);

    case TOK_CASE:
        return case_statement(p);

    case TOK_DEFAULT:
        return default_statement(p);

    case '{':
        return compound_statement(p);

    case '}':
        return NULL;

    default:
        if (consume(p, TOK_IDENT)) {
            if (nexttok(p, ':')) {
                /* unget identifier */
                ungettok(p);
                return labeled_statement(p);
            }
            /* unget identifier */
            ungettok(p);
        }
        return expression_statement(p);
    }
}

static struct ast_node *decl_identifier(struct parser *p)
{
    struct ast_node *tree = NULL;

    if (!consume(p, TOK_IDENT))
        return tree;

    tree = NEW_(NOD_DECL_IDENT);
    copy_token_text(p, tree);

    decl_set_ident(p, tree->sval);

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

    spec = type_specifier(p);
    if (!spec)
        return NULL;

    decl_begin(p, SYM_MEMBER);

    tree = NEW_(NOD_DECL_MEMBER);
    tree->l = spec;
    tree->r = struct_declarator_list(p);

    return tree;
}

/*
 * struct_declaration_list
 *     struct_decl
 *     struct_declaration_list ',' struct_decl
 */
static struct ast_node *struct_declaration_list(struct parser *p)
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

static struct ast_node *struct_or_union(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_STRUCT:
        decl_begin(p, SYM_TAG_STRUCT);
        tree = NEW_(NOD_SPEC_STRUCT);
        break;

    default:
        /* TODO error */
        break;
    }
    return tree;
}

static struct ast_node *struct_or_union_specifier(struct parser *p)
{
    struct ast_node *tree = NULL;

    tree = struct_or_union(p);
    tree->l = decl_identifier(p);

    decl_set_type(p, type_struct(tree->l->sval));

    if (!consume(p, '{')) {
        use_sym(p, tree->l);
        return tree;
    } else {
        define_sym(p, tree->l);
    }

    scope_begin(p);
    tree->r = struct_declaration_list(p);
    scope_end(p);

    expect(p, '}');

    return tree;
}

/*
 * enumerator
 *     TOK_IDENT
 *     TOK_IDENT '=' constant_expression
 */
static struct ast_node *enumerator(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *ident = NULL;

    ident = decl_identifier(p);
    if (!ident)
        return NULL;

    decl_begin(p, SYM_ENUMERATOR);

    tree = NEW_(NOD_DECL_ENUMERATOR);
    tree->l = ident;

    decl_set_type(p, type_int());
    define_sym(p, tree->l);

    if (consume(p, '='))
        tree->r = constant_expression(p);

    return tree;
}

/*
 * enumerator_list
 *     enumerator
 *     enumerator_list ',' enumerator
 */
static struct ast_node *enumerator_list(struct parser *p)
{
    struct ast_node *tree = NULL;

    for (;;) {
        struct ast_node *enu = enumerator(p);

        if (!enu)
            return tree;

        tree = new_node(NOD_LIST, tree, enu);

        if (!consume(p, ','))
            return tree;
    }
}

/*
 * enum_specifier
 *     TOK_ENUM '{' enumerator_list '}'
 *     TOK_ENUM TOK_IDENT '{' enumerator_list '}'
 *     TOK_ENUM TOK_IDENT
 */
static struct ast_node *enum_specifier(struct parser *p)
{
    struct ast_node *tree = NULL;

    expect(p, TOK_ENUM);

    decl_begin(p, SYM_TAG_ENUM);

    tree = NEW_(NOD_SPEC_ENUM);
    tree->l = decl_identifier(p);

    decl_set_type(p, type_enum(tree->l->sval));

    if (!consume(p, '{')) {
        use_sym(p, tree->l);
        return tree;
    } else {
        define_sym(p, tree->l);
    }

    tree->r = enumerator_list(p);

    expect(p, '}');

    return tree;
}

static struct ast_node *type_specifier(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_VOID:
        tree = NEW_(NOD_SPEC_VOID);
        decl_set_type(p, type_void());
        break;

    case TOK_CHAR:
        tree = NEW_(NOD_SPEC_CHAR);
        decl_set_type(p, type_char());
        break;

    case TOK_INT:
        tree = NEW_(NOD_SPEC_INT);
        decl_set_type(p, type_int());
        break;

    case TOK_STRUCT:
        ungettok(p);
        tree = struct_or_union_specifier(p);
        break;

    case TOK_ENUM:
        ungettok(p);
        tree = enum_specifier(p);
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

    spec = declaration_specifier(p);
    if (!spec)
        return NULL;

    decl_begin(p, SYM_PARAM);

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
        ungettok(p);
        tree->r = decl_identifier(p);
    } else {
        /* no identifier declared */
        return tree;
    }

    if (consume(p, '[')) {
        struct ast_node *array = NEW_(NOD_SPEC_ARRAY);

        if (consume(p, TOK_NUM))
            copy_token_ival(p, array);

        tree->l = array;
        expect(p, ']');

        decl_set_type(p, type_array(p->decl_type, array->ival));
    }

    if (consume(p, '(')) {
        struct ast_node *fn = NEW_(NOD_DECL_FUNC);

        decl_begin(p, SYM_FUNC);
        define_sym(p, tree->r);
        scope_begin(p);

        fn->l = tree;
        fn->r = param_decl_list(p);
        tree = fn;
        expect(p, ')');
    } else {
        /* non-function */
        define_sym(p, tree->r);
    }

    return tree;
}

static struct ast_node *pointer(struct parser *p)
{
    struct ast_node *tree = NULL;

    while (consume(p, '*')) {
        tree = new_node(NOD_SPEC_POINTER, tree, NULL);
        decl_set_type(p, type_ptr(p->decl_type));
    }

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

static struct ast_node *declaration_specifier(struct parser *p)
{
    struct ast_node *tree = type_specifier(p);
    return tree;
}

/*
 * declaration
 *     declaration_specifier ';'
 *     declaration_specifier init_declarator_list ';'
 */
static struct ast_node *declaration(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *spec = NULL;

    /* Returning NULL doesn't mean syntax error in function scopes
     * We can try to parse a statement instead */
    spec = declaration_specifier(p);
    if (!spec)
        return NULL;

    decl_begin(p, SYM_VAR);

    tree = NEW_(NOD_DECL);
    tree->l = spec;
    tree->r = init_declarator_list(p);

    if (nexttok(p, '{')) {
        /* is func definition */
        tree = new_node(NOD_FUNC_DEF, tree, compound_statement(p));
        scope_end(p);
        return tree;
    } else if (decl_is_func(p)) {
        /* is func prototype */
        scope_end(p);
    }

    expect(p, ';');

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

/*
 * translation_unit
 *     extern_declaration
 *     translation_unit extern_declaration
 */
static struct ast_node *translation_unit(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *decl = NULL;

    while (!consume(p, TOK_EOF)) {
        decl = extern_decl(p);

        if (!decl) {
            syntax_error(p, "unexpected token at external declaration");
            continue;
        }

        tree = new_node(NOD_LIST, tree, decl);
    }

    return tree;
}

struct ast_node *parse(struct parser *p)
{
    return translation_unit(p);
}
