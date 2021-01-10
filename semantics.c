#include <stdio.h>
#include <string.h>
#include "semantics.h"
#include "message.h"
#include "ast.h"

static int eval_(const struct ast_node *node, struct message_list *messages)
{
    /* TODO remove this */
    const struct position pos = {0};
    int l, r;

    if (!node)
        return 0;

    switch (node->kind) {

    case NOD_ADD:
        l = eval_(node->l, messages);
        r = eval_(node->r, messages);
        return l + r;

    case NOD_SUB:
        l = eval_(node->l, messages);
        r = eval_(node->r, messages);
        return l - r;

    case NOD_MUL:
        l = eval_(node->l, messages);
        r = eval_(node->r, messages);
        return l * r;

    case NOD_DIV:
        l = eval_(node->l, messages);
        r = eval_(node->r, messages);
        return l / r;

    case NOD_NUM:
        return node->ival;

    case NOD_DECL_IDENT:
    case NOD_IDENT:
        if (node->sym->kind != SYM_ENUMERATOR) {
            add_error(messages, "expression is not a constant expression", &pos);
            return 0;
        }
        return node->sym->mem_offset;

    default:
        add_error(messages, "expression is not a constant expression", &pos);
        return 0;
    }
}

static const struct data_type *promote_data_type(
        const struct ast_node *n1, const struct ast_node *n2)
{
    if (!n1 && !n2) {
        return type_void();
    }

    if (!n1) {
        return n2->type;
    }

    if (!n2) {
        return n1->type;
    }

    if (n1->type->kind > n2->type->kind) {
        return n1->type;
    } else {
        return n2->type;
    }
}

static int align_to(int pos, int align)
{
    return ((pos + align - 1) / align) * align;
}

static void compute_enum_size(struct symbol_table *table, struct symbol *enm)
{
    enm->mem_offset = get_size(type_int());
}

static void compute_struct_size(struct symbol_table *table, struct symbol *strc)
{
    struct symbol *sym;
    int total_offset = 0;
    /* inside of struct is one level upper than struct scope */
    const int struct_scope = strc->scope_level + 1;

    for (sym = strc; sym; sym = sym->next) {
        /* TODO support nested struct by checking scope level */
        if (sym->kind == SYM_MEMBER) {
            const int size  = sym->type->byte_size;
            const int align = sym->type->alignment;
            const int len   = sym->type->array_len;

            total_offset = align_to(total_offset, align);
            sym->mem_offset = total_offset;
            total_offset += len * size;
        }

        if (sym->kind == SYM_SCOPE_END && sym->scope_level == struct_scope)
            break;
    }

    strc->type->byte_size = align_to(total_offset, strc->type->alignment);
    strc->mem_offset = strc->type->byte_size;
}

static void compute_func_size(struct symbol_table *table, struct symbol *func)
{
    struct symbol *sym;
    int total_offset = 0;

    for (sym = func; sym; sym = sym->next) {
        if (is_param(sym) || is_local_var(sym)) {
            const int size  = get_size(sym->type);
            const int align = get_alignment(sym->type);
            const int len   = get_array_length(sym->type);

            total_offset = align_to(total_offset, align);
            total_offset += len * size;
            sym->mem_offset = total_offset;
        }

        if (sym->kind == SYM_TAG_STRUCT)
            compute_struct_size(table, sym);

        if (sym->kind == SYM_TAG_ENUM)
            compute_enum_size(table, sym);

        if (sym->kind == SYM_SCOPE_END && sym->scope_level == 1)
            break;
    }

    func->mem_offset = align_to(total_offset, 16);
}

static void add_symbol_size(struct symbol_table *table)
{
    struct symbol *sym;

    for (sym = table->head; sym; sym = sym->next) {
        if (sym->kind == SYM_FUNC)
            compute_func_size(table, sym);

        if (sym->kind == SYM_TAG_STRUCT)
            compute_struct_size(table, sym);

        if (sym->kind == SYM_TAG_ENUM)
            compute_enum_size(table, sym);
    }
}

static int check_symbol_usage(struct symbol_table *table, struct message_list *messages)
{
    struct symbol *sym;

    for (sym = table->head; sym; sym = sym->next) {
        /* TODO remove this */
        const struct position pos = {0};

        if (is_local_var(sym)) {
            /* TODO support check for struct variables */
            if (sym->type->kind == DATA_TYPE_STRUCT)
                continue;

            if (sym->is_redefined)
                add_error(messages, "redefinition of variable", &pos);

            if (!sym->is_defined && sym->is_used)
                add_error(messages, "use of undefined symbol", &pos);

            if (sym->type->kind == DATA_TYPE_VOID)
                add_error(messages, "variable has incomplete type 'void'", &pos);

            if (sym->is_defined && !sym->is_used)
                add_warning(messages, "unused variable", &pos);

            if (sym->is_defined && sym->is_used && !sym->is_initialized)
                add_warning(messages, "uninitialized variable used", &pos);
        }
        else if (is_enumerator(sym)) {
            if (!sym->is_defined && sym->is_used)
                add_error(messages, "use of undefined symbol", &pos);

            if (sym->is_assigned)
                add_error(messages, "expression is not assignable", &pos);
        }
        else if (is_func(sym)) {
            if (!sym->is_defined && sym->is_used)
                add_warning(messages, "implicit declaration of function", &pos);
        }
        else if (is_label(sym)) {
            if (sym->is_redefined)
                add_error(messages, "redefinition of label ''", &pos);

            if (!sym->is_defined && sym->is_used)
                add_error(messages, "use of undeclared label ''", &pos);
        }
    }
    return 0;
}

struct tree_context {
    struct message_list *messages;
    struct symbol *func_sym;
    int loop_depth;
    int switch_depth;
    int enum_value;
    int is_lvalue;
    int has_init;
};

static int find_case_value(struct ast_node *node)
{
    const struct symbol *sym = node->sym;
    const int val = node->l->ival;
    const int lv = node->sym->scope_level;

    /* search from sym->prev */
    for (sym = sym->prev; sym; sym = sym->prev) {
        if (sym->kind == SYM_CASE &&
                sym->mem_offset == val && sym->scope_level == lv)
            return 1;

        if (sym->kind == SYM_SWITCH_BEGIN && sym->scope_level == lv)
            break;
    }
    return 0;
}

static void check_tree_(struct ast_node *node, struct tree_context *ctx)
{
    /* TODO remove this */
    const struct position pos = {0};

    if (!node)
        return;

    switch (node->kind) {

    /* declaration */
    case NOD_DECL_INIT:
        ctx->has_init = node->r ? 1 : 0;
        check_tree_(node->l, ctx);
        ctx->has_init = 0;
        check_tree_(node->r, ctx);
        return;

    case NOD_DECL_IDENT:
        node->sym->is_initialized = ctx->has_init;
        if (node->sym->kind == SYM_FUNC)
            ctx->func_sym = node->sym;
        break;

    case NOD_ASSIGN:
        /* evaluate rvalue first to check a = a + 1; */
        check_tree_(node->r, ctx);
        ctx->is_lvalue = 1;
        check_tree_(node->l, ctx);
        ctx->is_lvalue = 0;
        return;

    case NOD_IDENT:
        if (ctx->is_lvalue && !node->sym->is_used)
            node->sym->is_initialized = 1;
        node->sym->is_assigned = ctx->is_lvalue;
        node->sym->is_used = 1;
        break;

    /* enum */
    case NOD_SPEC_ENUM:
        ctx->enum_value = 0;
        break;

    case NOD_DECL_ENUMERATOR:
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        if (node->r)
            ctx->enum_value = node->r->ival;
        node->l->sym->mem_offset = ctx->enum_value;
        ctx->enum_value++;
        return;

    /* loop */
    case NOD_FOR:
    case NOD_WHILE:
    case NOD_DOWHILE:
        ctx->loop_depth++;
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        ctx->loop_depth--;
        return;

    case NOD_FOR_PRE_COND:
        if (!node->r) {
            node->r = new_ast_node(NOD_NUM, NULL, NULL);
            node->r->ival = 1;
        }
        break;

    /* switch */
    case NOD_SWITCH:
        ctx->switch_depth++;
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        ctx->switch_depth--;
        return;

    case NOD_CASE:
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        node->sym->mem_offset = node->l->ival;
        if (find_case_value(node))
            add_error(ctx->messages, "duplicate case value", &pos);
        return;

    /* function */
    case NOD_DECL_FUNC:
        ctx->func_sym = NULL;
        break;

    case NOD_FUNC_DEF:
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        ctx->func_sym = NULL;
        return;

    /* break and continue */
    case NOD_BREAK:
        if (ctx->loop_depth == 0 && ctx->switch_depth == 0)
            add_error(ctx->messages, "'break' statement not in loop or switch statement", &pos);
        break;

    case NOD_CONTINUE:
        if (ctx->loop_depth == 0)
            add_error(ctx->messages, "'continue' statement not in loop statement", &pos);
        break;

    case NOD_RETURN:
        if (ctx->func_sym->type->kind == DATA_TYPE_VOID && node->l)
            add_error(ctx->messages, "function '' should not return a value", &pos);
        if (ctx->func_sym->type->kind != DATA_TYPE_VOID && !node->l)
            add_error(ctx->messages, "function '' should return a value", &pos);
        break;

    /* constant */
    case NOD_CONST_EXPR:
        node->ival = eval_(node->l, ctx->messages);
        break;

    case NOD_SIZEOF:
        node->ival = get_size(node->l->type);
        break;

    default:
        break;;
    }

    check_tree_(node->l, ctx);
    check_tree_(node->r, ctx);
}

static void check_tree_semantics(struct ast_node *tree, struct message_list *messages)
{
    struct tree_context ctx = {0};
    ctx.messages = messages;
    check_tree_(tree, &ctx);
}

static void add_node_type(struct ast_node *node)
{
    if (!node)
        return;

    add_node_type(node->l);
    add_node_type(node->r);

    switch (node->kind) {

    case NOD_ASSIGN:
    case NOD_ADD_ASSIGN:
    case NOD_DECL_INIT:
        node->type = node->l->type;
        break;

    case NOD_DEREF:
        node->type = promote_data_type(node->l, node->r);
        node->type = node->type->ptr_to;;
        break;

    case NOD_CALL:
        node->type = node->l->type;
        break;

    case NOD_STRUCT_REF:
        node->type = node->sym->type;
        break;

    /* nodes with symbol */
    case NOD_DECL_IDENT:
    case NOD_IDENT:
        node->type = node->sym->type;
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

    case NOD_TYPE_NAME:
        /* type already set in parser */
        break;

    default:
        node->type = promote_data_type(node->l, node->r);
        break;
    }
}

int semantic_analysis(struct ast_node *tree,
        struct symbol_table *table, struct message_list *messages)
{
    add_symbol_size(table);
    add_node_type(tree);

    check_tree_semantics(tree, messages);
    check_symbol_usage(table, messages);

    return 0;
}
