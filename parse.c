#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parse.h"

/* XXX */
#define NEW_(kind) new_node(kind, NULL, NULL)
static struct ast_node *new_node(enum ast_node_kind kind,
        struct ast_node *l, struct ast_node *r)
{
    return new_ast_node(kind, l, r);
}

static void type_name_or_identifier(struct parser *p);

static const struct token *gettok(struct parser *p)
{
    const int N = TOKEN_BUFFER_SIZE;

    if (p->head == p->curr) {
        p->curr = (p->curr + 1) % N;
        p->head = p->curr;
        get_next_token(p->lex, &p->tokbuf[p->curr]);
        type_name_or_identifier(p);
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

static void type_name_or_identifier(struct parser *p)
{
    struct token *tok = &p->tokbuf[p->curr];

    if (tok->kind == TOK_IDENT) {
        const struct symbol *sym = find_type_name_symbol(p->symtab, tok->text);
        if (sym)
            tok->kind = TOK_TYPE_NAME;
    }
}

static void syntax_error(struct parser *p, const char *msg, ...)
{
    va_list va;
    va_start(va, msg);
    {
        char buf[128] = {'\0'};
        vsprintf(buf, msg, va);

        if (!p->is_panic_mode) {
            const struct token *tok = current_token(p);
            add_error(p->diag, &tok->pos, buf);
            p->is_panic_mode = 1;
        }
    }
    va_end(va);
}

static void expect(struct parser *p, enum token_kind query)
{
    const struct token *tok = gettok(p);

    if (p->is_panic_mode)
        return;

    if (tok->kind != query)
        syntax_error(p, "expected '%c'", query);
}

static void expect_or_recover(struct parser *p, enum token_kind query)
{
    const struct token *tok = gettok(p);

    if (tok->kind == query)
        return;

    for (;;) {
        tok = gettok(p);
        if (tok->kind == query || tok->kind == TOK_EOF)
            break;
    }

    p->is_panic_mode = 0;
    return;
}

struct parser *new_parser(void)
{
    struct parser *p = calloc(1, sizeof(struct parser));
    int i;

    for (i = 0; i < TOKEN_BUFFER_SIZE; i++)
        init_token(&p->tokbuf[i]);

    p->lex = new_lexer();
    p->head = 0;
    p->curr = 0;

    return p;
}

void free_parser(struct parser *p)
{
    if (!p)
        return;
    free_lexer(p->lex);
    free(p);
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
    case NOD_MOD_ASSIGN:
    case NOD_SHL_ASSIGN:
    case NOD_SHR_ASSIGN:
    case NOD_OR_ASSIGN:
    case NOD_XOR_ASSIGN:
    case NOD_AND_ASSIGN:
        node->type = node->l->type;
        break;

    case NOD_SHL:
    case NOD_SHR:
        node->type = node->l->type;
        break;

    case NOD_CALL:
        /* TODO need function to get return type from function type */
        node->type = underlying(underlying(node->l->type));
        break;

    case NOD_CAST:
        if (node->l)
            node->type = node->l->type;
        break;

    case NOD_ADDR:
        node->type = type_pointer(node->l->type);
        break;

    case NOD_DEREF:
        node->type = underlying(node->l->type);
        if (!node->type)
            node->type = node->l->type;
        break;

    case NOD_STRUCT_REF:
        node->type = node->r->type;
        break;

    case NOD_COND:
        node->type = node->r->type;
        break;

    case NOD_COMMA:
        node->type = node->r->type;
        break;

    /* nodes with symbol */
    case NOD_DECL_IDENT:
    case NOD_STRING:
        node->type = node->sym->type;
        break;

    case NOD_IDENT:
        if (is_func(node->sym))
            node->type = type_pointer(node->sym->type);
        else
            node->type = node->sym->type;
        break;

    /* logical ops */
    case NOD_LOGICAL_OR:
    case NOD_LOGICAL_AND:
    case NOD_LOGICAL_NOT:
    case NOD_EQ:
    case NOD_NE:
        node->type = promote_type(node->l, node->r);
        if (is_pointer(node->type))
            node->type = type_long();
        else
            node->type = type_int();
        break;

    /* nodes without type */
    case NOD_DECL:
    case NOD_LIST:
    case NOD_COMPOUND:
        break;

    /* nodes with literal */
    case NOD_NUM:
        /* TODO improve long/int decision */
        node->type = node->ival >> 32 ? type_long() : type_int();
        break;

    /* pionter arithmetic */
    case NOD_SUB:
        if (is_pointer(node->l->type) && is_pointer(node->r->type))
            node->type = type_long();
        else
            node->type = promote_type(node->l, node->r);
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
    return typed_(node);
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

    case NOD_MOD:
        l = eval_const_expr(node->l, p);
        r = eval_const_expr(node->r, p);
        return l % r;

    case NOD_SHL:
        l = eval_const_expr(node->l, p);
        r = eval_const_expr(node->r, p);
        return l << r;

    case NOD_SHR:
        l = eval_const_expr(node->l, p);
        r = eval_const_expr(node->r, p);
        return l >> r;

    case NOD_SIZEOF:
    case NOD_NUM:
        return node->ival;

    case NOD_DECL_IDENT:
    case NOD_IDENT:
        if (node->sym->kind != SYM_ENUMERATOR) {
            add_error(p->diag, &node->pos, "expression is not a constant expression");
            return 0;
        }
        return node->sym->mem_offset;

    default:
        add_error(p->diag, &node->pos, "expression is not a constant expression");
        return 0;
    }
}

static struct ast_node *convert_(struct parser *p, struct ast_node *node)
{
    if (!node)
        return NULL;

    if (is_array(node->type) &&
        !p->is_sizeof_operand &&
        !p->is_addressof_operand &&
        !(p->is_array_initializer && node->kind == NOD_STRING)) {

        struct ast_node *tree = new_node_(NOD_CAST, tokpos(p));
        tree = branch_(tree, NULL, node);
        tree->type = type_pointer(underlying(node->type));

        return tree;
    }

    return node;
}

/* decl context */
static void define_sym(struct parser *p, struct ast_node *node)
{
    struct symbol *sym;

    sym = define_symbol(p->symtab, p->decl.ident, p->decl.kind, p->decl.type);

    if (p->decl.kind != SYM_TAG_STRUCT &&
        p->decl.kind != SYM_TAG_UNION &&
        p->decl.kind != SYM_TAG_ENUM) {
        sym->is_extern = p->decl.is_extern;
        sym->is_static = p->decl.is_static;
    }
    sym->pos = node->pos;

    node->sym = sym;
    node->type = node->sym->type;

    p->decl.sym = sym;
}

static void use_sym(struct parser *p, struct ast_node *ident, int sym_kind)
{
    struct symbol *sym;

    sym = use_symbol(p->symtab, ident->sval, sym_kind);
    ident->sym = sym;
    ident->type = ident->sym->type;
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

    sym = define_label_symbol(p->symtab, p->decl.ident);
    sym->pos = node->pos;

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

static void define_ellipsis(struct parser *p)
{
    define_ellipsis_symbol(p->symtab);
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

static int decl_is_func(struct parser *p)
{
    return p->decl.kind == SYM_FUNC;
}

static int decl_is_param(struct parser *p)
{
    return p->decl.kind == SYM_PARAM;
}

static void decl_reset_context(struct parser *p)
{
    const struct declaration ini = {0};
    p->decl = ini;
}

static int is_type_spec(int kind)
{
    switch (kind) {
    case TOK_VOID:
    case TOK_CHAR:
    case TOK_SHORT:
    case TOK_INT:
    case TOK_LONG:
    case TOK_SIGNED:
    case TOK_UNSIGNED:
    case TOK_STRUCT:
    case TOK_UNION:
    case TOK_ENUM:
    case TOK_TYPE_NAME:
        return 1;
    default:
        return 0;
    }
}

static int is_storage_class_spec(int kind)
{
    switch (kind) {
    case TOK_STATIC:
    case TOK_EXTERN:
    case TOK_TYPEDEF:
        return 1;
    default:
        return 0;
    }
}

static int is_type_qual(int kind)
{
    switch (kind) {
    case TOK_CONST:
        return 1;
    default:
        return 0;
    }
}

static int is_type_spec_qual(int kind)
{
    return is_type_spec(kind) || is_type_qual(kind);
}

static int is_start_of_decl(int kind)
{
    return is_type_spec_qual(kind) || is_storage_class_spec(kind);
}

static void default_to_int(struct parser *p)
{
    if (!p->decl.type) {
        const struct token *next = gettok(p);
        p->decl.type = type_int();
        add_warning(p->diag, &next->pos, "type specifier missing, defaults to 'int'");
        ungettok(p);
    }
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
        return convert_(p, typed_(tree));

    case TOK_IDENT:
        ungettok(p);
        tree = identifier(p);
        /* TODO may not need to switch symbol kind */
        if (nexttok(p, '('))
            use_sym(p, tree, SYM_FUNC);
        else
            use_sym(p, tree, SYM_VAR);
        return convert_(p, typed_(tree));

    case '(':
        tree = expression(p);
        expect(p, ')');
        return tree;

    default:
        syntax_error(p, "expected expression");
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

    tree = new_node_(NOD_ARG, tokpos(p));
    return branch_(tree, asgn, NULL);
}

/*
 * argument_expression_list
 *     argument_expression_list
 *     argument_expression_list ',' argument_expression
 */
static struct ast_node *argument_expression_list(struct parser *p)
{
    struct ast_node *tree = NULL, *list = NULL;
    int count = 0;

    for (;;) {
        struct ast_node *arg = argument_expression(p);

        if (!arg)
            break;

        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, arg);
        count++;

        if (!consume(p, ','))
            break;
    }

    if (tree)
        tree->ival = count;

    return tree;
}

static struct ast_node *struct_ref(struct parser *p, struct ast_node *strc)
{
    struct ast_node *member = NULL, *ref = NULL;
    ref = new_node_(NOD_STRUCT_REF, tokpos(p));

    member = identifier(p);
    use_member_sym(p, strc->type, member);
    typed_(member);

    ref = branch_(ref, strc, member);
    return convert_(p, ref);
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
            if (!nexttok(p, ')')) {
                args = argument_expression_list(p);
                call->ival = args ? args->ival : 0;
            }
            tree = branch_(call, tree, args);
            expect(p, ')');
            break;

        case '[':
            add = new_node_(NOD_ADD, tokpos(p));
            tree = branch_(add, tree, expression(p));
            expect(p, ']');
            deref = new_node_(NOD_DEREF, tokpos(p));
            tree = branch_(deref, tree, NULL);
            /* TODO find the best place to convert_ */
            tree = convert_(p, tree);
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
    struct ast_node *tree = NULL, *tname = NULL;

    if (consume(p, '(')) {
        const int next = peektok(p);

        if (is_type_spec_qual(next)) {
            tname = type_name(p);
            expect(p, ')');

            tree = new_node_(NOD_CAST, tokpos(p));
            return branch_(tree, tname, cast_expression(p));
        }
        else {
            /* unget '(' then try unary_expression */
            ungettok(p);
        }
    }

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
        num = new_node_num(-1, tokpos(p));
        tree = new_node_(NOD_MUL, tokpos(p));
        return branch_(tree, num, cast_expression(p));

    case '*':
        tree = new_node_(NOD_DEREF, tokpos(p));
        return branch_(tree, cast_expression(p), NULL);

    case '&':
        p->is_addressof_operand = 1;
        tree = new_node_(NOD_ADDR, tokpos(p));
        tree = branch_(tree, cast_expression(p), NULL);
        p->is_addressof_operand = 0;
        return tree;

    case '!':
        tree = new_node_(NOD_LOGICAL_NOT, tokpos(p));
        return branch_(tree, cast_expression(p), NULL);

    case '~':
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
            const int next = peektok(p);

            if (is_type_spec_qual(next)) {
                struct ast_node *tname = type_name(p);

                tree = new_node_(NOD_SIZEOF, tokpos(p));
                expect(p, ')');
                tree = branch_(tree, tname, NULL);
                tree->ival = get_size(tree->l->type);
                return tree;
            } else {
                /* unget '(' then try 'sizeof expression' */
                ungettok(p);
            }
        }
        p->is_sizeof_operand = 1;
        tree = new_node_(NOD_SIZEOF, tokpos(p));
        tree = branch_(tree, unary_expression(p), NULL);
        tree->ival = tree->l ? get_size(tree->l->type) : 0;
        p->is_sizeof_operand = 0;

        return tree;

    default:
        ungettok(p);
        return postfix_expression(p);
    }
}

/*
 * multiplicative_expression
 *     cast_expression
 *     multiplicative_expression '*' cast_expression
 *     multiplicative_expression '/' cast_expression
 */
static struct ast_node *multiplicative_expression(struct parser *p)
{
    struct ast_node *tree = cast_expression(p);
    struct ast_node *expr = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '*':
            expr = new_node_(NOD_MUL, tokpos(p));
            tree = branch_(expr, tree, cast_expression(p));
            break;

        case '/':
            expr = new_node_(NOD_DIV, tokpos(p));
            tree = branch_(expr, tree, cast_expression(p));
            break;

        case '%':
            expr = new_node_(NOD_MOD, tokpos(p));
            tree = branch_(expr, tree, cast_expression(p));
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
 * shift_expression
 *     additive_expression
 *     shift_expression TOK_SHL additive_expression
 *     shift_expression TOK_SHR additive_expression
 */
static struct ast_node *shift_expression(struct parser *p)
{
    struct ast_node *tree = additive_expression(p);
    struct ast_node *expr = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_SHL:
            expr = new_node_(NOD_SHL, tokpos(p));
            tree = branch_(expr, tree, additive_expression(p));
            break;

        case TOK_SHR:
            expr = new_node_(NOD_SHR, tokpos(p));
            tree = branch_(expr, tree, additive_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * relational_expression
 *     shift_expression
 *     relational_expression '<' shift_expression
 *     relational_expression '>' shift_expression
 *     relational_expression TOK_LE shift_expression
 *     relational_expression TOK_GE shift_expression
 */
static struct ast_node *relational_expression(struct parser *p)
{
    struct ast_node *tree = shift_expression(p);
    struct ast_node *rela = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '<':
            rela = new_node_(NOD_LT, tokpos(p));
            tree = branch_(rela, tree, shift_expression(p));
            break;

        case '>':
            rela = new_node_(NOD_GT, tokpos(p));
            tree = branch_(rela, tree, shift_expression(p));
            break;

        case TOK_LE:
            rela = new_node_(NOD_LE, tokpos(p));
            tree = branch_(rela, tree, shift_expression(p));
            break;

        case TOK_GE:
            rela = new_node_(NOD_GE, tokpos(p));
            tree = branch_(rela, tree, shift_expression(p));
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
    struct ast_node *equa = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_EQ:
            equa = new_node_(NOD_EQ, tokpos(p));
            tree = branch_(equa, tree, relational_expression(p));
            break;

        case TOK_NE:
            equa = new_node_(NOD_NE, tokpos(p));
            tree = branch_(equa, tree, relational_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * and_expression
 *     equality_expression
 *     and_expression '&' equality_expression
 */
static struct ast_node *and_expression(struct parser *p)
{
    struct ast_node *tree = equality_expression(p);
    struct ast_node *expr = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '&':
            expr = new_node_(NOD_AND, tokpos(p));
            tree = branch_(expr, tree, equality_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * exclusive_or_expression
 *     and_expression
 *     exclusive_or_expression '^' and_expression
 */
static struct ast_node *exclusive_or_expression(struct parser *p)
{
    struct ast_node *tree = and_expression(p);
    struct ast_node *expr = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '^':
            expr = new_node_(NOD_XOR, tokpos(p));
            tree = branch_(expr, tree, and_expression(p));
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
    struct ast_node *tree = exclusive_or_expression(p);
    struct ast_node *expr = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case '|':
            expr = new_node_(NOD_OR, tokpos(p));
            tree = branch_(expr, tree, exclusive_or_expression(p));
            break;

        default:
            ungettok(p);
            return tree;
        }
    }
}

/*
 * logical_and_expression
 *     inclusive_or_expression
 *     logical_and_expression TOK_LOGICAL_AND inclusive_or_expression
 */
static struct ast_node *logical_and_expression(struct parser *p)
{
    struct ast_node *tree = inclusive_or_expression(p);
    struct ast_node *logi = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_LOGICAL_AND:
            logi = new_node_(NOD_LOGICAL_AND, tokpos(p));
            tree = branch_(logi, tree, inclusive_or_expression(p));
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
    struct ast_node *logi = NULL;

    for (;;) {
        const struct token *tok = gettok(p);

        switch (tok->kind) {

        case TOK_LOGICAL_OR:
            logi = new_node_(NOD_LOGICAL_OR, tokpos(p));
            tree = branch_(logi, tree, logical_and_expression(p));
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
    struct ast_node *then_else = NULL, *cond = NULL, *then = NULL, *els = NULL;
    const struct token *tok = NULL;

    tree = logical_or_expression(p);
    tok = gettok(p);

    switch (tok->kind) {

    case '?':
        then = expression(p);
        expect(p, ':');
        els  = conditional_expression(p);

        then_else = new_node_(NOD_COND_THEN, tokpos(p));
        then_else = branch_(then_else, then, els);

        cond = new_node_(NOD_COND, tokpos(p));
        tree = branch_(cond, tree, then_else);
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
 *     unary_expression TOK_MOD_ASSIGN assignment_expression
 *     unary_expression TOK_SHL_ASSIGN assignment_expression
 *     unary_expression TOK_SHR_ASSIGN assignment_expression
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
        return branch_(asgn, tree, assignment_expression(p));
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

    case TOK_MOD_ASSIGN:
        asgn = new_node_(NOD_MOD_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

    case TOK_SHL_ASSIGN:
        asgn = new_node_(NOD_SHL_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

    case TOK_SHR_ASSIGN:
        asgn = new_node_(NOD_SHR_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

    case TOK_OR_ASSIGN:
        asgn = new_node_(NOD_OR_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

    case TOK_XOR_ASSIGN:
        asgn = new_node_(NOD_XOR_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

    case TOK_AND_ASSIGN:
        asgn = new_node_(NOD_AND_ASSIGN, tokpos(p));
        return branch_(asgn, tree, assignment_expression(p));

    default:
        ungettok(p);
        return tree;
    }
}

/*
 * expression
 *     assignment_expression
 *     expression ',' assignment_expression
 */
static struct ast_node *expression(struct parser *p)
{
    struct ast_node *tree = assignment_expression(p);
    struct ast_node *asgn = NULL;

    for (;;) {
        if (!consume(p, ','))
            return tree;

        asgn = assignment_expression(p);
        if (!asgn)
            return tree;

        tree = new_node(NOD_COMMA, tree, asgn);
        tree = typed_(tree);
    }
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
    tree = new_node_(NOD_BREAK, tokpos(p));
    expect(p, ';');

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
    tree = new_node_(NOD_CONTINUE, tokpos(p));
    expect(p, ';');

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
    tree = new_node_(NOD_RETURN, tokpos(p));

    if (!nexttok(p, ';'))
        expr = expression(p);
    expect(p, ';');

    return branch_(tree, expr, NULL);
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

    tree->r = statement(p);
    return tree;
}

/*
 * labeled_statement
 *     TOK_IDENT ':' statement
 */
static struct ast_node *labeled_statement(struct parser *p)
{
    /* this statement is actually for only goto statement
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
 * null_statement
 *     ';'
 */
static struct ast_node *null_statement(struct parser *p)
{
    expect(p, ';');
    return NULL;
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

    case ';':
        return null_statement(p);

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
    pointer(p);
    return direct_abstract_declarator(p);
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
        p->decl.is_const = 1;
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
    struct ast_node *list = NULL, *spec = NULL;

    for (;;) {
        const int next = peektok(p);

        if (is_type_qual(next))
            spec = type_qualifier(p);
        else
        if (is_type_spec(next))
            spec = type_specifier(p);
        else
            break;

        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, spec);
    }

    if (p->decl.kind != SYM_TAG_STRUCT &&
        p->decl.kind != SYM_TAG_UNION &&
        p->decl.kind != SYM_TAG_ENUM) {
        set_const(p->decl.type, p->decl.is_const);
        set_unsigned(p->decl.type, p->decl.is_unsigned);
        p->decl.is_const = 0;
        p->decl.is_unsigned = 0;
    }

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
    tree->type = p->decl.type;

    return tree;
}

static struct ast_node *decl_identifier(struct parser *p)
{
    struct ast_node *tree = NULL;

    if (consume(p, TOK_IDENT)) {
        /* need to add token pos after reading an identifier */
        tree = new_node_(NOD_DECL_IDENT, tokpos(p));
        copy_token_text(p, tree);
        p->decl.ident = tree->sval;
    } else {
        tree = new_node_(NOD_DECL_IDENT, tokpos(p));
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
    struct ast_node *tree = NULL;

    if (nexttok(p, ':')) {
        /* unnamed bit field */
        struct ast_node *decl = decl_identifier(p);
        define_sym(p, decl);
    }
    else {
        tree = declarator(p);
    }

    if (consume(p, ':')) {
        struct ast_node *expr = constant_expression(p);
        p->decl.sym->is_bitfield = 1;
        p->decl.sym->bit_width = expr->ival;
        free_ast_node(expr);
    }

    return tree;
}

/*
 * struct_declarator_list
 *     struct_declarator
 *     struct_declarator_list ',' struct_declarator
 */
static struct ast_node *struct_declarator_list(struct parser *p)
{
    struct ast_node *tree = NULL, *list = NULL;

    for (;;) {
        struct data_type *spec = p->decl.type;
        struct ast_node *decl = struct_declarator(p);
        p->decl.type = spec;

        if (!decl)
            return tree;

        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, decl);

        if (!consume(p, ','))
            return tree;
    }
}

/*
 * struct_declaration
 *     specifier_qualifier_list struct_declarator_list ';'
 */
static struct ast_node *struct_declaration(struct parser *p)
{
    struct ast_node *tree = NULL, *spec = NULL;

    decl_reset_context(p);

    spec = specifier_qualifier_list(p);
    if (!spec)
        return NULL;

    p->decl.kind = SYM_MEMBER;

    tree = new_node_(NOD_DECL_MEMBER, tokpos(p));
    tree = branch_(tree, spec, struct_declarator_list(p));

    expect(p, ';');
    return tree;
}

/*
 * struct_declaration_list
 *     struct_declaration
 *     struct_declaration_list struct_declaration
 */
static struct ast_node *struct_declaration_list(struct parser *p)
{
    struct ast_node *tree = NULL, *list = NULL;
    const struct declaration tmp = p->decl;

    for (;;) {
        const int next = peektok(p);

        if (is_type_spec_qual(next)) {
            struct ast_node *decl = struct_declaration(p);
            list = new_node_(NOD_LIST, tokpos(p));
            tree = branch_(list, tree, decl);
        }
        else if (next == '}' || next == TOK_EOF) {
            break;
        }
        else {
            const struct token *tok = gettok(p);
            if (tok->kind == TOK_IDENT)
                syntax_error(p, "unknown type name '%s'", tok->text);
            else
                syntax_error(p, "type name requires a specifier or qualifier");
            ungettok(p);
            break;
        }
    }

    p->decl = tmp;
    return tree;
}

static struct ast_node *struct_or_union(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_STRUCT:
        p->decl.kind = SYM_TAG_STRUCT;
        tree = NEW_(NOD_SPEC_STRUCT);
        break;

    case TOK_UNION:
        p->decl.kind = SYM_TAG_UNION;
        tree = NEW_(NOD_SPEC_UNION);
        break;

    default:
        /* TODO error */
        break;
    }
    return tree;
}

/*
 * struct_or_union_specifier
 *     struct_or_union TK_IDENT '{' struct_declaration_list '}'
 *     struct_or_union '{' struct_declaration_list '}'
 *     struct_or_union TK_IDENT
 */
static struct ast_node *struct_or_union_specifier(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *ident = NULL;
    int sym_kind;

    tree = struct_or_union(p);
    ident = decl_identifier(p);
    tree->l = ident;

    /* struct or union sym */
    sym_kind = p->decl.kind;

    if (!consume(p, '{')) {
        /* define an object of struct type */
        use_sym(p, ident, sym_kind);
        p->decl.type = ident->sym->type;
        return tree;
    } else {
        /* define a struct type */
        if (sym_kind == SYM_TAG_STRUCT)
            p->decl.type = type_struct();
        else
            p->decl.type = type_union();
        define_sym(p, ident);
    }

    begin_scope(p);
    tree->r = struct_declaration_list(p);
    end_scope(p);

    expect(p, '}');

    if (sym_kind == SYM_TAG_STRUCT)
        compute_struct_size(ident->type->sym);
    else
        compute_union_size(ident->type->sym);

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

    p->decl.kind = SYM_ENUMERATOR;

    tree = NEW_(NOD_DECL_ENUMERATOR);
    tree->l = ident;

    p->decl.type = type_int();
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

    p->decl.kind = SYM_TAG_ENUM;

    tree = NEW_(NOD_SPEC_ENUM);
    ident = decl_identifier(p);

    if (!consume(p, '{')) {
        /* define an object of enum type */
        use_sym(p, ident, SYM_TAG_ENUM);
        p->decl.type = ident->sym->type;
        return tree;
    } else {
        /* define an enum type */
        p->decl.type = type_enum();
        define_sym(p, ident);
    }

    tree->r = enumerator_list(p);

    expect(p, '}');
    compute_enum_size(ident->sym);

    return tree;
}

static struct ast_node *type_specifier(struct parser *p)
{
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);

    switch (tok->kind) {

    case TOK_VOID:
        tree = NEW_(NOD_SPEC_VOID);
        p->decl.type = type_void();
        break;

    case TOK_CHAR:
        tree = NEW_(NOD_SPEC_CHAR);
        p->decl.type = type_char();
        break;

    case TOK_SHORT:
        tree = NEW_(NOD_SPEC_SHORT);
        p->decl.type = type_short();
        break;

    case TOK_INT:
        tree = NEW_(NOD_SPEC_INT);
        p->decl.type = type_int();
        break;

    case TOK_LONG:
        tree = NEW_(NOD_SPEC_LONG);
        p->decl.type = type_long();
        break;

    case TOK_SIGNED:
        tree = NEW_(NOD_SPEC_SIGNED);
        break;

    case TOK_UNSIGNED:
        tree = NEW_(NOD_SPEC_UNSIGNED);
        p->decl.is_unsigned = 1;
        break;

    case TOK_STRUCT:
    case TOK_UNION:
        ungettok(p);
        tree = struct_or_union_specifier(p);
        break;

    case TOK_ENUM:
        ungettok(p);
        tree = enum_specifier(p);
        break;

    case TOK_TYPE_NAME:
        {
            struct symbol *sym = find_type_name_symbol(p->symtab, tok->text);

            if (sym) {
                tree = new_node_(NOD_SPEC_TYPE_NAME, tokpos(p));
                p->decl.type = type_type_name(sym);
                break;
            }
            else {
                /* assert */
            }
        }
        break;

    default:
        /* assert */
        break;
    }

    return NULL;
}

/* parameter_declaration
 *     declaration_specifiers declarator
 *     declaration_specifiers abstract_declarator
 *     declaration_specifiers
 */
static struct ast_node *parameter_declaration(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *spec = NULL, *decl = NULL;

    decl_reset_context(p);
    spec = declaration_specifiers(p);

    /* void parameter */
    if (is_void(p->decl.type) && !nexttok(p, '*'))
        return NULL;

    p->decl.kind = SYM_PARAM;
    tree = new_node_(NOD_DECL_PARAM, tokpos(p));
    decl = declarator(p);

    /* 6.7.6.3 A declaration of a parameter as "array of type"
     * shall be adjusted to "qualified pointer to type" */
    convert_array_to_pointer(p->decl.type);

    return branch_(tree, spec, decl);
}

/*
 * parameter_list
 *     parameter_declaration
 *     parameter_list ',' parameter_declaration
 */
static struct ast_node *parameter_list(struct parser *p)
{
    struct ast_node *tree = NULL, *list = NULL;

    for (;;) {
        struct ast_node *param = parameter_declaration(p);

        if (!param)
            return tree;

        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, param);

        if (!consume(p, ','))
            return tree;
        if (nexttok(p, TOK_ELLIPSIS))
            return tree;
    }
}

/*
 * parameter_type_list
 *     parameter_list
 *     parameter_type_list ',' TOK_ELLIPSIS
 */
static struct ast_node *parameter_type_list(struct parser *p)
{
    struct ast_node *tree = NULL, *list = NULL, *elli = NULL;
    const struct declaration tmp = p->decl;

    tree = parameter_list(p);

    if (consume(p, TOK_ELLIPSIS)) {
        define_ellipsis(p);

        elli = new_node_(NOD_SPEC_ELLIPSIS, tokpos(p));
        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, elli);

        if (elli)
            p->func_sym->is_variadic = 1;
    }

    p->decl = tmp;
    return tree;
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
        p->decl.type = type_array(p->decl.type);
        tree->type = p->decl.type;

        if (expr)
            set_array_length(p->decl.type, expr->ival);
    }

    return tree;
}

/*
 * direct_declarator
 *     TOK_IDENT
 *     '(' declarator ')'
 *     direct_declarator '[' constant_expression ']'
 *     direct_declarator '[' ']'
 *     direct_declarator '(' parameter_type_list ')'
 *     direct_declarator '(' identifier_list ')'
 *     direct_declarator '(' ')'
 */
static struct ast_node *direct_declarator(struct parser *p)
{
    struct ast_node *tree = NULL;
    struct ast_node *ident = NULL;
    struct data_type *placeholder = NULL;

    if (consume(p, '(')) {
        struct data_type *tmp = p->decl.type;
        placeholder = type_void();
        p->decl.type = placeholder;

        tree = declarator(p);
        expect(p, ')');

        p->decl.type = tmp;
    }

    if (nexttok(p, TOK_IDENT) || decl_is_param(p)) {
        ident = decl_identifier(p);
        tree = ident;
    }

    if (nexttok(p, '['))
        array(p);

    if (consume(p, '(')) {
        if (!p->decl.sym) {
            /* function */
            /* functions are externnal by default */
            if (!p->decl.is_extern && !p->decl.is_static)
                p->decl.is_extern = 1;

            p->decl.type = type_function(p->decl.type);
            p->decl.kind = SYM_FUNC;
            define_sym(p, ident);
            p->func_sym = ident->sym;

            begin_scope(p);
            if (!nexttok(p, ')'))
                parameter_type_list(p);
            expect(p, ')');
        }
        else {
            /* function pointer */
            p->decl.type = type_function(p->decl.type);
            /* link function sym to type */
            set_symbol(p->decl.type, p->decl.sym);

            begin_scope(p);
            if (!nexttok(p, ')'))
                /* TODO consider saving this tree to better place */
                tree->r = parameter_type_list(p);
            end_scope(p);

            expect(p, ')');
        }
    } else {
        /* variable, parameter, member, typedef */
        if (ident)
            define_sym(p, ident);
    }

    if (placeholder) {
        copy_data_type(placeholder, p->decl.type);
        /* decl.type needs to re-point to sym type */
        p->decl.type = p->decl.sym->type;
    }

    return tree;
}

/* pointer
 *     '*'
 *     '*' type_qualifier_list
 *     '*' pointer
 *     '*' type_qualifier_list pointer
 */
static struct ast_node *pointer(struct parser *p)
{
    struct ast_node *tree = NULL, *ptr = NULL;

    while (consume(p, '*')) {
        /* we treat as if the first '*' is associated with type specifier */
        ptr = new_node_(NOD_SPEC_POINTER, tokpos(p));
        tree = branch_(ptr, tree, NULL);
        p->decl.type = type_pointer(p->decl.type);
    }

    return tree;
}

/* declarator
 *     pointer direct_declarator
 *     direct_declarator
 */
static struct ast_node *declarator(struct parser *p)
{
    pointer(p);
    return direct_declarator(p);
}

static struct ast_node *string_initializer(struct parser *p)
{
    struct ast_node *list = NULL, *init = NULL, *num = NULL;
    struct ast_node *tree = NULL;
    const struct token *tok = gettok(p);
    const char *ch = tok->text;
    int count = 0;

    do {
        num = new_node_num(*ch, tokpos(p));
        init = new_node_(NOD_INIT, tokpos(p));
        init = branch_(init, NULL, num);
        init->ival = count++;
        init->type = underlying(p->init_type);

        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, init);
        /* check null character at last since it needs to be added */
    } while (*ch++);

    if (has_unkown_array_length(p->init_type))
        set_array_length(p->init_type, count);

    return tree;
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
    struct ast_node *init = NULL;

    p->is_array_initializer = is_array(p->init_type);
    if (consume(p, '{')) {
        init = initializer_list(p);
        expect(p, '}');
    }
    else if (is_array(p->init_type) && nexttok(p, TOK_STRING_LITERAL)) {
        if (!is_char(underlying(p->init_type))) {
            /* TODO improve message */
            syntax_error(p, "initializing wide char array with non-wide string literal");
        }
        /* initializing array of char with string literal */
        init = string_initializer(p);
    }
    else {
        init = assignment_expression(p);
    }
    p->is_array_initializer = 0;

    tree = new_node_(NOD_INIT, tokpos(p));
    tree = branch_(tree, NULL, init);
    tree->type = p->init_type;

    return tree;
}

static struct data_type *initializer_child_type(struct parser *p,
        struct data_type *parent)
{
    if (is_array(parent)) {
        return underlying(parent);
    }
    else if (is_struct_or_union(parent)) {
        p->init_sym = first_member(symbol_of(parent));
        return p->init_sym->type;
    }
    else {
        return parent;
    }
}

static struct data_type *initializer_next_type(struct parser *p,
        struct data_type *parent)
{
    if (is_array(parent)) {
        return p->init_type;
    }
    else if (is_struct_or_union(parent)) {
        p->init_sym = next_member(p->init_sym);
        return p->init_sym ? p->init_sym->type : NULL;
    }
    else {
        return p->init_type;
    }
}

static int initializer_byte_offset(struct parser *p,
        struct data_type *parent, int index)
{
    if (is_array(parent))
        return get_size(p->init_type) * index;
    else if (is_struct_or_union(parent))
        return p->init_sym ? p->init_sym->mem_offset : 0;
    else
        return 0;
}

/*
 * initializer_list
 *     initializer
 *     initializer_list ',' initializer
 */
static struct ast_node *initializer_list(struct parser *p)
{
    struct ast_node *tree = NULL;
    int count = 0;

    struct data_type *parent = p->init_type;
    p->init_type = initializer_child_type(p, parent);

    for (;;) {
        struct ast_node *init = NULL, *list = NULL;

        init = initializer(p);

        if (!init)
            break;

        /* byte offset from the beginning of the list */
        init->ival = initializer_byte_offset(p, parent, count);
        p->init_type = initializer_next_type(p, parent);
        count++;

        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, init);

        if (!consume(p, ','))
            break;
    }

    p->init_type = parent;

    if (has_unkown_array_length(p->init_type))
        set_array_length(p->init_type, count);

    return tree;
}

/*
static struct ast_node *clone_tree(struct ast_node *node)
{
    struct ast_node *n;
    if (!node)
        return NULL;

    n = malloc(sizeof(struct ast_node));
    *n = *node;

    n->l = clone_tree(node->l);
    n->r = clone_tree(node->r);

    return n;
}
*/

/* init_declarator
 *     declarator
 *     declarator '=' initializer
 */
static struct ast_node *init_declarator(struct parser *p)
{
    struct ast_node *decl = NULL;

    decl = declarator(p);

    if (consume(p, '=')) {
        p->init_type = p->decl.sym->type;
        p->init_sym = symbol_of(p->init_type);

        decl->l = initializer(p);

        p->init_type = NULL;
        p->init_sym = NULL;
    }

    return typed_(decl);
}

/*
 * init_declarator_list
 *     init_declarator
 *     init_declarator_list ',' init_declarator
 */
static struct ast_node *init_declarator_list(struct parser *p)
{
    struct ast_node *tree = NULL, *list = NULL;

    for (;;) {
        struct data_type *spec = p->decl.type;
        struct ast_node *init = init_declarator(p);
        p->decl.type = spec;

        if (!init)
            return tree;

        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, init);

        if (!consume(p, ','))
            return tree;
    }
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
        p->decl.is_typedef = 1;
        break;

    case TOK_EXTERN:
        tree = new_node_(NOD_DECL_EXTERN, tokpos(p));
        p->decl.is_extern = 1;
        break;

    case TOK_STATIC:
        tree = new_node_(NOD_DECL_STATIC, tokpos(p));
        p->decl.is_static = 1;
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
    struct ast_node *tree = NULL;
    struct ast_node *list = NULL, *spec = NULL;

    for (;;) {
        const int next = peektok(p);

        if (is_storage_class_spec(next))
            spec = storage_class_specifier(p);
        else
        if (is_type_qual(next))
            spec = type_qualifier(p);
        else
        if (is_type_spec(next))
            spec = type_specifier(p);
        else
            break;

        list = new_node_(NOD_LIST, tokpos(p));
        tree = branch_(list, tree, spec);
    }

    default_to_int(p);

    if (p->decl.kind != SYM_TAG_STRUCT &&
        p->decl.kind != SYM_TAG_UNION &&
        p->decl.kind != SYM_TAG_ENUM) {
        set_const(p->decl.type, p->decl.is_const);
        set_unsigned(p->decl.type, p->decl.is_unsigned);
        /* TODO improve this by storing decl before entering struct */
        p->decl.is_const = 0;
        p->decl.is_unsigned = 0;
    }

    /* TODO improve this by storing decl before entering struct */
    if (p->decl.is_typedef)
        p->decl.kind = SYM_TYPEDEF;
    else
        p->decl.kind = SYM_VAR;

    return NULL;
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
    spec = declaration_specifiers(p);

    tree = new_node_(NOD_DECL, tokpos(p));
    tree = branch_(tree, spec, init_declarator_list(p));

    /* TODO temp for new_tree */
    if (!tree->r) {
        expect_or_recover(p, ';');
        return NULL;
    }

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
        /* is func prototype. unflag its (re)definition */
        p->func_sym->is_defined = 0;
        p->func_sym->is_redefined = 0;
        end_scope(p);
        p->func_sym = NULL;
    }

    expect_or_recover(p, ';');
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
        const int next = peektok(p);

        if (is_start_of_decl(next)) {
            struct ast_node *decl = declaration(p);
            tree = new_node(NOD_LIST, tree, decl);
        } else {
            break;
        }
    }

    return tree;
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
        const int next = peektok(p);

        if (next == '}' || next == TOK_EOF)
            return tree;

        tree = new_node(NOD_LIST, tree, statement(p));
    }
}

static struct ast_node *extern_decl(struct parser *p)
{
    const int next = peektok(p);

    if (is_start_of_decl(next)) {
        return declaration(p);
    } else {
        const struct token *tok = gettok(p);

        if (tok->kind == TOK_IDENT)
            syntax_error(p, "unknown type name '%s'", tok->text);
        else
            syntax_error(p, "unexpected token");

        /* recover */
        for (;;) {
            tok = gettok(p);

            if (is_start_of_decl(tok->kind) || tok->kind == TOK_EOF) {
                ungettok(p);
                p->is_panic_mode = 0;
                break;
            }
        }
        return NULL;
    }
}

/*
 * translation_unit
 *     extern_declaration
 *     translation_unit extern_declaration
 */
static struct ast_node *translation_unit(struct parser *p)
{
    struct ast_node *tree = NULL;

    while (!consume(p, TOK_EOF)) {
        struct ast_node *decl = extern_decl(p);

        if (!decl)
            /* TODO temp for new_tree */
            continue;

        tree = new_node(NOD_LIST, tree, decl);
    }

    return tree;
}

struct ast_node *parse_text(struct parser *p, const char *text,
        struct symbol_table *symtab, struct diagnostic *diag)
{
    if (!text)
        return NULL;

    if (!symtab || !diag)
        return NULL;

    p->symtab = symtab;
    p->diag = diag;
    set_source_text(p->lex, text);

    return translation_unit(p);
}
