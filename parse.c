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
        p->curr = (p->curr + 1) % N;
        p->head = p->curr;
        lex_get_token(&p->lex, &p->tokbuf[p->curr]);
    } else {
        p->curr = (p->curr + 1) % N;
    }

    return &p->tokbuf[p->curr];
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

static const struct position *tokpos(const struct parser *p)
{
    const struct token *tok = current_token(p);
    return &tok->pos;
}

static struct ast_node *new_node_(enum ast_node_kind kind, const struct position *pos)
{
    struct ast_node *node = new_ast_node(kind, NULL, NULL);
    node->pos = *pos;
    return node;
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

    p->decl_kind = 0;
    p->decl_ident = NULL;
    p->decl_type = NULL;

    p->enum_value = 0;
    p->func_sym = NULL;

    p->is_typedef = 0;
    p->is_extern = 0;
    p->is_static = 0;
    p->is_const = 0;
    p->is_panic_mode = 0;

    p->init_type = NULL;
    p->init_sym = NULL;

    return p;
}

void free_parser(struct parser *p)
{
    free(p);
}

/* type */
static void type_set(struct ast_node *node, struct data_type *type)
{
    node->type = type;
}

static void type_from_sym(struct ast_node *node)
{
    node->type = node->sym->type;
}

static struct data_type *promote_type(struct ast_node *n1, struct ast_node *n2)
{
    if (!n1 && !n2)
        return type_void();
    if (!n1)
        return n2->type;
    if (!n2)
        return n1->type;

    return promote(n1->type, n2->type);
}

static struct ast_node *typed_(struct ast_node *node)
{
    if (!node)
        return NULL;

    switch (node->kind) {

    case NOD_ASSIGN:
    case NOD_ADD_ASSIGN:
    case NOD_SUB_ASSIGN:
    case NOD_MUL_ASSIGN:
    case NOD_DIV_ASSIGN:
    case NOD_DECL_INIT:
        node->type = node->l->type;
        break;

    case NOD_CALL:
        node->type = node->l->type;
        break;

    case NOD_DEREF:
        node->type = underlying(node->l->type);
        if (!node->type)
            node->type = node->l->type;
        break;

    case NOD_STRUCT_REF:
        node->type = node->r->type;
        break;

    /* nodes with symbol */
    case NOD_DECL_IDENT:
    case NOD_IDENT:
        node->type = node->sym->type;
        break;

    /* nodes without type */
    case NOD_DECL:
    case NOD_LIST:
    case NOD_COMPOUND:
        break;

    /* nodes with literal */
    case NOD_NUM:
        node->type = type_int();
        break;

    case NOD_STRING:
        node->type = type_ptr(type_char());
        break;

    case NOD_SIZEOF:
        node->type = type_int();
        break;

    case NOD_PREINC:
    case NOD_PREDEC:
    case NOD_POSTINC:
    case NOD_POSTDEC:
        node->type = promote_type(node->l, node->r);
        break;

    default:
        node->type = promote_type(node->l, node->r);
        break;
    }

    return node;
}

static struct ast_node *branch_(struct ast_node *node,
        struct ast_node *l, struct ast_node *r)
{
    node->l = l;
    node->r = r;
    return typed_(node);
}

static struct ast_node *new_node_num(int num, const struct position *pos)
{
    struct ast_node *node = new_node_(NOD_NUM, pos);
    node->ival = num;
    return node;
}

static int eval_const_expr(const struct ast_node *node, struct parser *p)
{
    int l, r;

    if (!node)
        return 0;

    switch (node->kind) {

    case NOD_ADD:
        l = eval_const_expr(node->l, p);
        r = eval_const_expr(node->r, p);
        return l + r;

    case NOD_SUB:
        l = eval_const_expr(node->l, p);
        r = eval_const_expr(node->r, p);
        return l - r;

    case NOD_MUL:
        l = eval_const_expr(node->l, p);
        r = eval_const_expr(node->r, p);
        return l * r;

    case NOD_DIV:
        l = eval_const_expr(node->l, p);
        r = eval_const_expr(node->r, p);
        return l / r;

    case NOD_NUM:
        return node->ival;

    case NOD_DECL_IDENT:
    case NOD_IDENT:
        if (node->sym->kind != SYM_ENUMERATOR) {
            add_error2(p->msg, &node->pos, "expression is not a constant expression");
            return 0;
        }
        return node->sym->mem_offset;

    default:
        add_error2(p->msg, &node->pos, "expression is not a constant expression");
        return 0;
    }
}

/* decl context */
static void define_sym(struct parser *p, struct ast_node *node)
{
    struct symbol *sym;
    struct data_type *decl_type;

    /* to avoid typedef of typedef, typedef always refers actual type */
    if (p->decl_kind == SYM_TYPEDEF && is_type_name(p->decl_type))
        decl_type = original(p->decl_type);
    else
        decl_type = p->decl_type;

    sym = define_symbol(p->symtab, p->decl_ident, p->decl_kind, decl_type);
    /* >> TODO make func */
    sym->is_extern = p->is_extern;
    sym->is_static = p->is_static;
    p->is_extern = 0;
    p->is_static = 0;
    p->is_const = 0;
    /* << make func */
    sym->pos = node->pos;

    node->sym = sym;
    type_from_sym(node);
}

static void use_sym(struct parser *p, struct ast_node *ident, int sym_kind)
{
    struct symbol *sym;

    sym = use_symbol(p->symtab, ident->sval, sym_kind);
    ident->sym = sym;
    type_from_sym(ident);
}

static void use_member_sym(struct parser *p,
        const struct data_type *struct_type, struct ast_node *node)
{
    const char *member_name = node->sval;

    node->sym = use_struct_member_symbol(p->symtab, symbol_of(struct_type), member_name);
}

static void define_case(struct parser *p, struct ast_node *node, int kind, int case_value)
{
    struct symbol *sym;

    sym = define_case_symbol(p->symtab, kind, case_value);
    /* TODO come up with better place to put this or even
     * consider removing pos from sym */
    sym->pos = node->pos;

    node->sym = sym;
}

static void define_label(struct parser *p, struct ast_node *node)
{
    struct symbol *sym;

    sym = define_label_symbol(p->symtab, p->decl_ident);
    node->sym = sym;
}

static void use_label(struct parser *p, struct ast_node *node)
{
    if (!node)
        return;

    if (node->kind == NOD_GOTO) {
        struct ast_node *label = node->l;
        label->sym = use_label_symbol(p->symtab, label->sval);
    }
    use_label(p, node->l);
    use_label(p, node->r);
}

static void define_string(struct parser *p, struct ast_node *node)
{
    struct symbol *sym;

    sym = define_string_symbol(p->symtab, node->sval);
    node->sym = sym;
}

static void begin_scope(struct parser *p)
{
    symbol_scope_begin(p->symtab);
}

static void end_scope(struct parser *p)
{
    symbol_scope_end(p->symtab);
}

static void begin_switch(struct parser *p)
{
    symbol_switch_begin(p->symtab);
}

static void end_switch(struct parser *p)
{
    symbol_switch_end(p->symtab);
}

static void decl_set_kind(struct parser *p, int decl_kind)
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

static void begin_typedef(struct parser *p)
{
    p->is_typedef = 1;
}

static void end_typedef(struct parser *p)
{
    p->is_typedef = 0;
}

static int decl_is_typedef(struct parser *p)
{
    return p->is_typedef == 1;
}

static void begin_extern(struct parser *p)
{
    p->is_extern = 1;
}

static void end_extern(struct parser *p)
{
    p->is_extern = 0;
}

static void begin_static(struct parser *p)
{
    p->is_static = 1;
}

static void end_static(struct parser *p)
{
    p->is_static = 0;
}

static void decl_reset_context(struct parser *p)
{
    p->decl_kind = 0;
    p->decl_ident = NULL;
    p->decl_type = NULL;
    end_typedef(p);
    end_extern(p);
    end_static(p);
}

/*
 * forward declarations
 */
static struct ast_node *statement(struct parser *p);
static struct ast_node *expression(struct parser *p);
static struct ast_node *unary_expression(struct parser *p);
static struct ast_node *assignment_expression(struct parser *p);
static struct ast_node *initializer_list(struct parser *p);
static struct ast_node *decl_identifier(struct parser *p);
static struct ast_node *type_name(struct parser *p);
static struct ast_node *pointer(struct parser *p);
static struct ast_node *type_specifier(struct parser *p);
static struct ast_node *declaration_specifiers(struct parser *p);
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

    tree = new_node_(NOD_IDENT, tokpos(p));
    copy_token_text(p, tree);

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
        tree = new_node_(NOD_NUM, tokpos(p));
        copy_token_text(p, tree);
        copy_token_ival(p, tree);
        return typed_(tree);

    case TOK_STRING_LITERAL:
        tree = new_node_(NOD_STRING, tokpos(p));
        copy_token_text(p, tree);
        define_string(p, tree);
        return typed_(tree);

    case TOK_IDENT:
        ungettok(p);
        tree = identifier(p);
        if (nexttok(p, '('))
            use_sym(p, tree, SYM_FUNC);
        else
            use_sym(p, tree, SYM_VAR);
        return typed_(tree);

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

static struct ast_node *struct_ref(struct parser *p, struct ast_node *strc)
{
    struct ast_node *member = NULL, *ref = NULL;
    member = identifier(p);
    use_member_sym(p, strc->type, member);
    typed_(member);

    ref = new_node_(NOD_STRUCT_REF, tokpos(p));
    return branch_(ref, strc, member);
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
        struct ast_node *deref = NULL, *add = NULL;
        struct ast_node *call = NULL, *args = NULL, *incdec = NULL;
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '.':
            tree = struct_ref(p, tree);
            break;

        case TOK_POINTER:
            deref = new_node_(NOD_DEREF, tokpos(p));
            tree = branch_(deref, tree, NULL);

            tree = struct_ref(p, tree);
            break;

        case '(':
            call = new_node_(NOD_CALL, tokpos(p));
            if (!nexttok(p, ')'))
                args = argument_expression_list(p);
            tree = branch_(call, tree, args);
            expect(p, ')');
            break;

        case '[':
            add = new_node_(NOD_ADD, tokpos(p));
            tree = branch_(add, tree, expression(p));
            expect(p, ']');
            deref = new_node_(NOD_DEREF, tokpos(p));
            tree = branch_(deref, tree, NULL);
            break;

        case TOK_INC:
            incdec = new_node_(NOD_POSTINC, tokpos(p));
            tree = branch_(incdec, tree, NULL);
            break;

        case TOK_DEC:
            incdec = new_node_(NOD_POSTDEC, tokpos(p));
            tree = branch_(incdec, tree, NULL);
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * cast_expression
 *     unary_expression
 *     '(' type_name ')' cast_expression
 */
static struct ast_node *cast_expression(struct parser *p)
{
    return unary_expression(p);
}

/*
 * unary_expression
 *     postfix_expression
 *     TOK_INC unary_expression
 *     TOK_DEC unary_expression
 *     unary_operator cast_expression
 *     TOK_SIZEOF unary_expression
 *     TOK_SIZEOF '(' type_name ')'
 */
static struct ast_node *unary_expression(struct parser *p)
{
    struct ast_node *tree = NULL, *num = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case '+':
        return cast_expression(p);

    case '-':
        num = new_node_(NOD_NUM, tokpos(p));
        num->ival = -1;
        tree = new_node_(NOD_MUL, tokpos(p));
        return branch_(tree, num, cast_expression(p));

    case '*':
        tree = new_node_(NOD_DEREF, tokpos(p));
        return branch_(tree, cast_expression(p), NULL);

    case '&':
        tree = new_node_(NOD_ADDR, tokpos(p));
        return branch_(tree, cast_expression(p), NULL);

    case '!':
        tree = new_node_(NOD_NOT, tokpos(p));
        return branch_(tree, cast_expression(p), NULL);

    case TOK_INC:
        tree = new_node_(NOD_PREINC, tokpos(p));
        return branch_(tree, unary_expression(p), NULL);

    case TOK_DEC:
        tree = new_node_(NOD_PREDEC, tokpos(p));
        return branch_(tree, unary_expression(p), NULL);

    case TOK_SIZEOF:
        if (consume(p, '(')) {
            struct ast_node *tname = type_name(p);
            if (tname) {
                tree = new_node_(NOD_SIZEOF, tokpos(p));
                expect(p, ')');
                return branch_(tree, tname, NULL);
            } else {
                /* unget '(' */
                ungettok(p);
            }
        }
        tree = new_node_(NOD_SIZEOF, tokpos(p));
        return branch_(tree, unary_expression(p), NULL);

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
    struct ast_node *expr = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '*':
            expr = new_node_(NOD_MUL, tokpos(p));
            tree = branch_(expr, tree, unary_expression(p));
            break;

        case '/':
            expr = new_node_(NOD_DIV, tokpos(p));
            tree = branch_(expr, tree, unary_expression(p));
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
    struct ast_node *expr = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '+':
            expr = new_node_(NOD_ADD, tokpos(p));
            tree = branch_(expr, tree, multiplicative_expression(p));
            break;

        case '-':
            expr = new_node_(NOD_SUB, tokpos(p));
            tree = branch_(expr, tree, multiplicative_expression(p));
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
    struct ast_node *tree = NULL;
    struct ast_node *then_else = NULL, *then = NULL, *els = NULL;
    const struct token *tok = NULL;

    tree = logical_or_expression(p);

    tok = gettok(p);

    switch (tok->kind) {

    case '?':
        then = expression(p);
        expect(p, ':');
        els  = conditional_expression(p);

        then_else = new_node(NOD_COND_THEN, then, els);
        tree = new_node(NOD_COND, tree, then_else);
        break;

    default:
        ungettok(p);
        break;
    }
    return tree;
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
static struct ast_node *assignment_expression(struct parser *p);
static struct ast_node *assign_(struct parser *p, int node_kind, struct ast_node *lval)
{
    struct ast_node *asgn = new_node_(node_kind, tokpos(p));
    return branch_(asgn, lval, assignment_expression(p));
}

static struct ast_node *assignment_expression(struct parser *p)
{
    struct ast_node *tree = conditional_expression(p);
    struct ast_node *asgn = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case '=':
        /*
        asgn = new_node_(NOD_ASSIGN, tokpos(p));
        retrun branch_(asgn, tree, assignment_expression(p));
        */
        return assign_(p, NOD_ASSIGN, tree);

    case TOK_ADD_ASSIGN:
        asgn = new_node_(NOD_ADD_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

    case TOK_SUB_ASSIGN:
        asgn = new_node_(NOD_SUB_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

    case TOK_MUL_ASSIGN:
        asgn = new_node_(NOD_MUL_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

    case TOK_DIV_ASSIGN:
        asgn = new_node_(NOD_DIV_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

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
    struct ast_node *tree = NULL;
    struct ast_node *expr = conditional_expression(p);

    if (!expr)
        return NULL;

    tree = new_node_(NOD_CONST_EXPR, tokpos(p));
    tree = branch_(tree, expr, NULL);
    tree->ival = eval_const_expr(expr, p);

    return tree;
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

    begin_scope(p);
    tree = new_node(NOD_COMPOUND, NULL, NULL);
    tree->l = declaration_list(p);
    tree->r = statement_list(p);
    end_scope(p);

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
    if (!cond)
        cond = new_node_num(1, tokpos(p));

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
    /* check usage of label symbol in semantics as we need to see all
     * label declarations in advance */
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

    begin_switch(p);
    body = statement(p);
    end_switch(p);

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
    struct position valpos;

    expect(p, TOK_CASE);
    {
        /* get the position of the case value */
        const struct token *tok = gettok(p);
        valpos = tok->pos;
        ungettok(p);
    }
    expr = constant_expression(p);
    expect(p, ':');

    tree = new_node_(NOD_CASE, &valpos);
    tree = branch_(tree, expr, NULL);
    define_case(p, tree, SYM_CASE, expr->ival);

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
    struct position defpos;
    const int no_case_value = 0;

    expect(p, TOK_DEFAULT);
    {
        /* get the position of default */
        defpos = *tokpos(p);
    }
    expect(p, ':');

    tree = new_node_(NOD_DEFAULT, &defpos);
    tree = branch_(tree, NULL, NULL);
    define_case(p, tree, SYM_DEFAULT, no_case_value);

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
    decl_reset_context(p);

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
/* direct_abstract_declarator
 *     '(' abstract_declarator ')'
 *     '[' ']'
 *     '[' constant_expression ']'
 *     direct_abstract_declarator '[' ']'
 *     direct_abstract_declarator '[' constant_expression ']'
 *     '(' ')'
 *     '(' parameter_type_list ')'
 *     direct_abstract_declarator '(' ')'
 *     direct_abstract_declarator '(' parameter_type_list ')'
 */
static struct ast_node *direct_abstract_declarator(struct parser *p)
{
    struct ast_node *tree = NULL;

    return tree;
}

/*
 * abstract_declarator
 *     pointer
 *     direct_abstract_declarator
 *     pointer direct_abstract_declarator
 */
static struct ast_node *abstract_declarator(struct parser *p)
{
    struct ast_node *tree = NEW_(NOD_DECLARATOR);

    tree->l = pointer(p);
    tree->r = direct_abstract_declarator(p);

    return tree;
}

/*
 * type_qualifier
 *     TOK_CONST
 *     TOK_VOLATILE
 */
static struct ast_node *type_qualifier(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_CONST:
        tree = new_node_(NOD_QUAL_CONST, tokpos(p));
        p->is_const = 1;
        break;

    default:
        ungettok(p);
        break;
    }

    return tree;
}

/*
 * specifier_qualifier_list
 *     type_specifier specifier_qualifier_list
 *     type_specifier
 *     type_qualifier specifier_qualifier_list
 *     type_qualifier
 */
static struct ast_node *specifier_qualifier_list(struct parser *p)
{
    struct ast_node *tree = NULL;
    tree = type_specifier(p);
    return tree;
}

/*
 * type_name
 *     specifier_qualifier_list
 *     specifier_qualifier_list abstract_declarator
 */
static struct ast_node *type_name(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *spec = NULL;

    spec = specifier_qualifier_list(p);
    if (!spec)
        return NULL;

    tree = NEW_(NOD_TYPE_NAME);
    tree->l = spec;
    tree->r = abstract_declarator(p);

    type_set(tree, p->decl_type);
    return tree;
}

static struct ast_node *decl_identifier(struct parser *p)
{
    struct ast_node *tree = NULL;

    if (consume(p, TOK_IDENT)) {
        tree = new_node_(NOD_DECL_IDENT, tokpos(p));
        copy_token_text(p, tree);
        decl_set_ident(p, tree->sval);
    } else {
        tree = new_node_(NOD_DECL_IDENT, tokpos(p));
        decl_set_ident(p, NULL);
    }

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
 * struct_declaration
 *     spec_qual_list struct_declarator_list
 */
static struct ast_node *struct_declaration(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *spec = NULL;

    spec = type_specifier(p);
    if (!spec)
        return NULL;

    decl_set_kind(p, SYM_MEMBER);

    tree = NEW_(NOD_DECL_MEMBER);
    tree->l = spec;
    tree->r = struct_declarator_list(p);

    return tree;
}

/*
 * struct_declaration_list
 *     struct_declaration
 *     struct_declaration_list ',' struct_declaration
 */
static struct ast_node *struct_declaration_list(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct data_type *tmp = p->decl_type;

    for (;;) {
        struct ast_node *decl = struct_declaration(p);

        if (!decl)
            break;

        tree = new_node(NOD_LIST, tree, decl);
        expect(p, ';');
    }

    p->decl_type = tmp;
    return tree;
}

static struct ast_node *struct_or_union(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_STRUCT:
        decl_set_kind(p, SYM_TAG_STRUCT);
        tree = NEW_(NOD_SPEC_STRUCT);
        break;

    default:
        /* TODO error */
        break;
    }
    return tree;
}

/* struct_or_union_specifier
 *     struct_or_union TK_IDENT '{' struct_declaration_list '}'
 *     struct_or_union '{' struct_declaration_list '}'
 *     struct_or_union TK_IDENT
 */
static struct ast_node *struct_or_union_specifier(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *ident = NULL;

    tree = struct_or_union(p);
    ident = decl_identifier(p);
    tree->l = ident;

    if (!consume(p, '{')) {
        /* define an object of struct type */
        use_sym(p, ident, SYM_TAG_STRUCT);
        decl_set_type(p, ident->sym->type);
        return tree;
    } else {
        /* define a struct type */
        decl_set_type(p, type_struct(ident->sval));
        define_sym(p, ident);
    }

    begin_scope(p);
    tree->r = struct_declaration_list(p);
    end_scope(p);

    expect(p, '}');
    compute_struct_size(ident->type->sym);

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

    if (!nexttok(p, TOK_IDENT))
        return NULL;
    ident = decl_identifier(p);

    decl_set_kind(p, SYM_ENUMERATOR);

    tree = NEW_(NOD_DECL_ENUMERATOR);
    tree->l = ident;

    decl_set_type(p, type_int());
    define_sym(p, ident);

    if (consume(p, '=')) {
        tree->r = constant_expression(p);
        p->enum_value = tree->r->ival;
    }

    ident->sym->mem_offset = p->enum_value++;
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

    p->enum_value = 0;

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
    struct ast_node *ident = NULL;

    expect(p, TOK_ENUM);

    decl_set_kind(p, SYM_TAG_ENUM);

    tree = NEW_(NOD_SPEC_ENUM);
    ident = decl_identifier(p);

    if (!consume(p, '{')) {
        /* define an object of enum type */
        use_sym(p, ident, SYM_TAG_ENUM);
        decl_set_type(p, ident->sym->type);
        return tree;
    } else {
        /* define an enum type */
        decl_set_type(p, type_enum(ident->sval));
        define_sym(p, ident);
    }

    tree->r = enumerator_list(p);

    expect(p, '}');
    compute_enum_size(ident->sym);

    return tree;
}

static struct ast_node *type_specifier(struct parser *p)
{
    struct symbol *sym = NULL;
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

    case TOK_IDENT:
        sym = find_type_name_symbol(p->symtab, tok->text);

        if (sym) {
            tree = new_node_(NOD_SPEC_TYPE_NAME, tokpos(p));
            decl_set_type(p, type_type_name(tok->text, sym));
        } else {
            /* unget identifier */
            ungettok(p);
        }
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

    spec = declaration_specifiers(p);
    if (!spec)
        return NULL;

    decl_set_kind(p, SYM_PARAM);

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

/*
 * array
 *     '[' ']'
 *     '[' constant_expression ']'
 *     array '[' constant_expression ']'
 */
static struct ast_node *array(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *expr = NULL;

    if (consume(p, '[')) {
        tree = new_node_(NOD_SPEC_ARRAY, tokpos(p));
        if (!nexttok(p, ']'))
            expr = constant_expression(p);
        expect(p, ']');

        tree = branch_(tree, expr, array(p));
        decl_set_type(p, type_array(p->decl_type));
        type_set(tree, p->decl_type);

        if (expr)
            set_array_length(p->decl_type, expr->ival);
    }

    return tree;
}

static struct ast_node *direct_declarator(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *ident = NULL;

    tree = NEW_(NOD_DECL_DIRECT);

    if (consume(p, '(')) {
        tree->l = declarator(p);
        expect(p, ')');
    }

    if (nexttok(p, TOK_IDENT)) {
        ident = decl_identifier(p);
        tree->r = ident;
    } else {
        /* no identifier declared */
        return tree;
    }

    tree->l = array(p);

    if (consume(p, '(')) {
        /* function */
        struct ast_node *fn = NEW_(NOD_DECL_FUNC);
        decl_set_kind(p, SYM_FUNC);
        define_sym(p, ident);
        p->func_sym = ident->sym;

        begin_scope(p);
        fn->l = tree;
        fn->r = param_decl_list(p);
        tree = fn;
        expect(p, ')');
    } else {
        /* variable, parameter, member, typedef */
        define_sym(p, ident);
    }

    return typed_(tree);
}

static struct ast_node *pointer(struct parser *p)
{
    struct ast_node *tree = NULL;

    while (consume(p, '*')) {
        /* we treat as if the first '*' is associated with type specifier */
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

    return typed_(tree);
}

/*
 * initializer
 *     assignment_expression
 *     '{' initializer_list '}'
 *     '{' initializer_list ',' '}'
 */
static struct ast_node *initializer(struct parser *p)
{
    /* ',' at the end of list is handled by initializer_list */
    struct ast_node *tree = NULL;
    struct ast_node *init;

    if (consume(p, '{')) {
        init = initializer_list(p);
        expect(p, '}');
    } else {
        init = assignment_expression(p);
    }

    tree = new_node_(NOD_INIT, tokpos(p));
    tree = branch_(tree, NULL, init);
    type_set(tree, p->init_type);

    return tree;
}

/* TODO consider adding types, symbols and index at the same place */
static struct data_type *initializer_child_type(struct parser *p,
        struct data_type *parent)
{
    struct data_type *type = parent;

    if (is_array(type)) {
        return underlying(type);
    }
    else if (is_struct(type)) {
        struct symbol *sym = symbol_of(type);
        p->init_sym = sym->next->next;
        return p->init_sym->type;
    }
    else {
        return type;
    }
}

static struct data_type *initializer_next_type(struct parser *p,
        struct data_type *parent)
{
    struct data_type *type = p->init_type;

    if (is_array(parent)) {
        return type;
    }
    else if (is_struct(parent)) {
        p->init_sym = p->init_sym->next;
        return p->init_sym->type;
    }
    else {
        return type;
    }
}

/*
 * initializer_list
 *     initializer
 *     initializer_list ',' initializer
 */
static struct ast_node *initializer_list(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *init_list;
    int count = 0;

    struct data_type *parent = p->init_type;
    p->init_type = initializer_child_type(p, parent);

    for (;;) {
        struct ast_node *init = NULL, *list = NULL;
        init = initializer(p);

        if (!init)
            break;

        p->init_type = initializer_next_type(p, parent);

        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, init);
        count++;

        if (!consume(p, ','))
            break;
    }

    p->init_type = parent;

    init_list = new_node_(NOD_INIT_LIST, tokpos(p));
    init_list = branch_(init_list, tree, NULL);
    type_set(init_list, p->init_type);

    if (has_unkown_array_length(p->init_type))
        set_array_length(p->init_type, count);

    return init_list;
}

static struct ast_node *init_declarator(struct parser *p)
{
    struct ast_node *tree = NEW_(NOD_DECL_INIT);

    tree->l = declarator(p);

    if (consume(p, '=')) {
        p->init_type = p->decl_type;
        p->init_sym = symbol_of(p->init_type);

        tree->r = initializer(p);

        p->init_type = NULL;
        p->init_sym = NULL;
    }

    return typed_(tree);
}

/*
 * init_declarator_list
 *     init_declarator
 *     init_declarator_list ',' init_declarator
 */
static struct ast_node *init_declarator_list(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct data_type *spec = p->decl_type;

    for (;;) {
        struct ast_node *init = init_declarator(p);

        if (!init)
            break;

        decl_set_type(p, spec);
        tree = new_node(NOD_LIST, tree, init);

        if (!consume(p, ','))
            break;
    }

    decl_set_type(p, spec);
    return tree;
}

/*
 * storage_class_specifier
 *     TOK_TYPEDEF
 *     TOK_EXTERN
 *     TOK_STATIC
 *     TOK_AUTO
 *     TOK_REGISTER
 */
static struct ast_node *storage_class_specifier(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_TYPEDEF:
        tree = new_node_(NOD_DECL_TYPEDEF, tokpos(p));
        decl_set_type(p, type_void());
        begin_typedef(p);
        break;

    case TOK_EXTERN:
        tree = new_node_(NOD_DECL_EXTERN, tokpos(p));
        begin_extern(p);
        break;

    case TOK_STATIC:
        tree = new_node_(NOD_DECL_STATIC, tokpos(p));
        begin_static(p);
        break;

    default:
        ungettok(p);
        break;
    }

    return tree;
}

/*
 * declaration_specifiers
 *     storage_class_specifier
 *     storage_class_specifier declaration_specifiers
 *     type_specifier
 *     type_specifier declaration_specifiers
 *     type_qualifier
 *     type_qualifier declaration_specifiers
 */
static struct ast_node *declaration_specifiers(struct parser *p)
{
    /* this function should return NULL when there is no type spec
     * so decl know we're not parsing a decl anymore and move on to
     * parsing statements. */
    struct ast_node *tree = NULL, *decl = NULL;
    struct ast_node *list, *stor, *qual, *spec;

    for (;;) {
        stor = storage_class_specifier(p);
        if (stor) {
            list = new_node_(NOD_LIST, tokpos(p));
            tree = branch_(list, tree, stor);
        }

        qual = type_qualifier(p);
        if (qual) {
            list = new_node_(NOD_LIST, tokpos(p));
            tree = branch_(list, tree, qual);
        }

        spec = type_specifier(p);
        if (spec) {
            list = new_node_(NOD_LIST, tokpos(p));
            tree = branch_(list, tree, spec);
        }

        if (!stor && !qual && !spec)
            break;
    }

    if (!tree)
        return NULL;

    set_const(p->decl_type, p->is_const);
    if (decl_is_typedef(p))
        decl_set_kind(p, SYM_TYPEDEF);
    else
        decl_set_kind(p, SYM_VAR);

    decl = new_node_(NOD_DECL_SPEC, tokpos(p));
    return branch_(decl, tree, NULL);
}

/*
 * declaration
 *     declaration_specifiers ';'
 *     declaration_specifiers init_declarator_list ';'
 */
static struct ast_node *declaration(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *spec = NULL;

    decl_reset_context(p);
    /* Returning NULL doesn't mean syntax error in function scopes
     * We can try to parse a statement instead */
    spec = declaration_specifiers(p);
    if (!spec)
        return NULL;

    tree = new_node_(NOD_DECL, tokpos(p));
    tree = branch_(tree, spec, init_declarator_list(p));

    if (nexttok(p, '{')) {
        /* is func definition */
        struct ast_node *stmt = compound_statement(p);
        tree = new_node(NOD_FUNC_DEF, tree, stmt);
        use_label(p, stmt);
        end_scope(p);
        compute_func_size(p->func_sym);
        p->func_sym = NULL;
        return tree;
    } else if (decl_is_func(p)) {
        /* is func prototype */
        end_scope(p);
        p->func_sym = NULL;
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
