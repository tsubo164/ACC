#include <stdio.h>
#include <string.h>
#include "semantics.h"

static int is_orig_used(const struct symbol *sym)
{
    if (!sym)
        return 0;
    if (has_origin(sym))
        return sym->orig->is_used;
    return sym->is_used;
}

static int is_orig_initialized(const struct symbol *sym)
{
    if (!sym)
        return 0;
    if (has_origin(sym))
        return sym->orig->is_initialized;
    return sym->is_initialized;
}

static int check_symbol_usage(struct symbol_table *table, struct diagnostic *diag)
{
    struct symbol *sym;

    for (sym = table->head; sym; sym = sym->next) {
        if (is_local_var(sym)) {
            /* TODO support check for struct variables */
            if (is_struct_or_union(sym->type))
                continue;

            if (sym->is_defined && !sym->is_used && is_origin(sym))
                add_warning(diag, &sym->pos, "unused variable '%s'", sym->name);
        }
        else if (is_global_var(sym)) {
            if (is_static(sym) && !sym->is_used)
                add_warning(diag, &sym->pos, "unused variable '%s'", sym->name);
        }
        else if (is_func(sym)) {
            if (sym->is_defined && !is_orig_used(sym) && is_static(sym))
                add_warning(diag, &sym->pos, "unused function '%s'", sym->name);
        }
        else if (is_case(sym)) {
            if (sym->is_redefined)
                add_error(diag, &sym->pos, "duplicate case value '%d'", sym->mem_offset);
        }
        else if (is_default(sym)) {
            if (sym->is_redefined)
                add_error(diag, &sym->pos, "multiple default labels in one switch");
        }
        else if (is_label(sym)) {
            if (sym->is_defined && !sym->is_used)
                add_warning(diag, &sym->pos, "unused label '%s'", sym->name);
        }
    }
    return 0;
}

struct tree_context {
    struct diagnostic *diag;
    const struct data_type *func_type;
    const struct parameter *param;
    int loop_depth;
    int switch_depth;
    int is_lvalue;
    int is_union;
};

static void check_initializer(struct ast_node *node, struct tree_context *ctx);

static int is_integer_zero(struct ast_node *node)
{
    if (is_integer(node->type) &&
        node->kind == NOD_NUM &&
        node->ival == 0)
        return 1;
    return 0;
}

static char type_name1[128] = {'\0'};
static char type_name2[128] = {'\0'};

static void check_init_scalar(struct ast_node *node, struct tree_context *ctx)
{
    struct data_type *t1, *t2;

    if (!node)
        return;

    switch (node->kind) {

    case NOD_INIT:
        t1 = node->l->type;
        t2 = node->r->type;

        /* integer zero to pointer */
        if (is_pointer(t1) && is_integer_zero(node->r))
            return;

        if (!is_compatible(t1, t2)) {
            make_type_name(t1, type_name1);
            make_type_name(t2, type_name2);
            if (is_pointer(t1))
                add_error(ctx->diag, &node->pos,
                        "incompatible pointer types initializing '%s' with an expression of type '%s'",
                        type_name1, type_name2);
            else
                add_error(ctx->diag, &node->pos,
                        "initializing '%s' with an expression of incompatible type '%s'",
                        type_name1, type_name2);
        }
        break;

    default:
        break;
    }
}

static void check_init_array_element(struct ast_node *node, struct tree_context *ctx)
{
    if (!node)
        return;

    switch (node->kind) {

    case NOD_INIT:
        if (is_void(node->l->type)) {
            add_error(ctx->diag, &node->pos,
                    "excess elements in array initializer");
            break;
        }

        check_initializer(node, ctx);
        break;

    case NOD_LIST:
        check_init_array_element(node->l, ctx);
        check_init_array_element(node->r, ctx);
        break;

    default:
        break;
    }
}

static void check_init_struct_members(struct ast_node *node, struct tree_context *ctx)
{
    if (!node)
        return;

    switch (node->kind) {

    case NOD_INIT:
        if (is_void(node->l->type)) {
            add_error(ctx->diag, &node->pos,
                    "excess elements in %s initializer",
                    ctx->is_union ? "union" : "struct");
            break;
        }

        check_initializer(node, ctx);
        break;

    case NOD_LIST:
        check_init_struct_members(node->l, ctx);
        check_init_struct_members(node->r, ctx);
        break;

    default:
        break;
    }
}

static void check_initializer(struct ast_node *node, struct tree_context *ctx)
{
    struct ast_node *desi = node ? node->l : NULL;
    struct ast_node *expr = node ? node->r : NULL;

    if (!desi || !expr)
        return;

    if (is_array(desi->type)) {
        struct tree_context new_ctx = *ctx;

        check_init_array_element(expr, &new_ctx);
    }
    else if(is_struct_or_union(desi->type)) {
        if (expr->kind == NOD_INIT || expr->kind == NOD_LIST) {
            /* initialized by member initializer */
            struct tree_context new_ctx = *ctx;
            new_ctx.is_union = is_union(desi->type);

            check_init_struct_members(expr, &new_ctx);
        }
        else {
            /* initialized by struct object */
            check_init_scalar(node, ctx);
        }
    }
    else {
        check_init_scalar(node, ctx);
    }
}

static void check_tree_(struct ast_node *node, struct tree_context *ctx)
{
    if (!node)
        return;

    switch (node->kind) {

    /* declaration */
    case NOD_DECL_IDENT:
        /* has initializer or is a global variable */
        node->sym->is_initialized = node->l || is_global_var(node->sym);

        if (is_function(node->type))
            ctx->func_type = node->type;

        if (is_incomplete(node->sym->type) &&
                (is_local_var(node->sym) || is_global_var(node->sym))) {
            make_type_name(node->sym->type, type_name1);
            add_error(ctx->diag, &node->pos, "variable has incomplete type '%s'",
                    type_name1);
            return;
        }

        if (is_label(node->sym)) {
            if (node->sym->is_redefined)
                add_error(ctx->diag, &node->pos, "redefinition of label '%s'",
                        node->sym->name);
        }

        if (is_local_var(node->sym)) {
            if (node->sym->is_redefined)
                add_error(ctx->diag, &node->pos, "redefinition of '%s'",
                        node->sym->name);
        }

        if (node->l)
            check_initializer(node->l, ctx);

        break;

    /* TODO add sub_assign, ... */
    case NOD_ASSIGN:
        /* evaluate rvalue first to check a = a + 1; */
        check_tree_(node->r, ctx);
        ctx->is_lvalue = 1;
        check_tree_(node->l, ctx);
        ctx->is_lvalue = 0;

        if (node->l->kind == NOD_DEREF) {
            if (is_const(node->type))
                add_error(ctx->diag, &node->pos,
                        "read-only variable is not assignable");
        }
        else if (node->l->kind == NOD_IDENT) {
            if (is_const(node->type))
                add_error(ctx->diag, &node->pos,
                        "cannot assign to variable '%s' with const-qualified",
                        node->l->sym->name);
        }
        else {
            /* TODO assert? */
        }

        /* integer zero to pointer */
        if (is_pointer(node->l->type) && is_integer_zero(node->r))
            return;

        if (node->l && node->r && !is_compatible(node->l->type, node->r->type)) {
            make_type_name(node->l->type, type_name1);
            make_type_name(node->r->type, type_name2);
            if (is_pointer(node->type))
                add_error(ctx->diag, &node->pos,
                        "incompatible pointer types assigning to '%s' from '%s'",
                        type_name1, type_name2);
            else
                add_error(ctx->diag, &node->pos,
                        "assigning to '%s' from incompatible type '%s'",
                        type_name1, type_name2);
        }
        return;

    case NOD_IDENT:
        if (ctx->is_lvalue && !node->sym->is_used)
            node->sym->is_initialized = 1;
        node->sym->is_assigned = ctx->is_lvalue;
        node->sym->is_used = 1;
        /* update orig->is_used */
        if (node->sym->orig)
            node->sym->orig->is_used = 1;

        {
            struct symbol *sym = node->sym;

            if (is_local_var(sym) || is_global_var(sym)) {
                if (sym->is_defined && sym->is_used && !is_orig_initialized(sym))
                    /* array, struct, union will not be treated as uninitialized */
                    if (!is_array(sym->type) && !is_struct_or_union(sym->type))
                        add_warning(ctx->diag, &node->pos,
                                "variable '%s' is uninitialized when used here", sym->name);

                if (!sym->is_defined && sym->is_used)
                    add_error(ctx->diag, &node->pos,
                            "use of undeclared identifier '%s'", sym->name);
            }
            else if (is_func(sym)) {
                if (!sym->is_defined && sym->is_used && !is_extern(sym) && !is_static(sym))
                    add_warning(ctx->diag, &node->pos,
                            "implicit declaration of function '%s'", sym->name);
            }
            else if (is_enumerator(sym)) {
                if (sym->is_assigned)
                    add_error(ctx->diag, &node->pos, "expression is not assignable");
            }
            else if (is_label(sym)) {
                if (!sym->is_defined && sym->is_used)
                    add_error(ctx->diag, &node->pos, "use of undeclared label '%s'",
                            sym->name);
            }
        }
        break;

    /* struct */
    case NOD_STRUCT_REF:
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        if (!is_struct_or_union(node->l->type)) {
            make_type_name(node->l->type, type_name1);
            add_error(ctx->diag, &node->pos,
                    "member reference base type '%.32s' is not a structure or union",
                    type_name1);
            return;
        }
        if (is_incomplete(node->l->type)) {
            make_type_name(node->l->type, type_name1);
            add_error(ctx->diag, &node->pos,
                    "incomplete definition of type '%.32s'",
                    type_name1);
            return;
        }
        if (!node->r->sym->is_defined) {
            make_type_name(node->l->type, type_name1);
            add_error(ctx->diag, &node->pos,
                    "no member named '%.32s' in '%.32s'",
                    node->r->sym->name, type_name1);
        }
        return;

    case NOD_DEREF:
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        if (!underlying(node->l->type))
            add_error(ctx->diag, &node->pos, "indirection requires pointer operand");
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

    /* switch */
    case NOD_SWITCH:
        ctx->switch_depth++;
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        ctx->switch_depth--;
        return;

    /* function */
    case NOD_FUNC_DEF:
        ctx->func_type = NULL;
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        ctx->func_type = NULL;
        return;

    case NOD_CALL:
        {
            const struct parameter *tmp = ctx->param;

            check_tree_(node->l, ctx);
            ctx->param = first_param_(underlying(node->l->type));
            check_tree_(node->r, ctx);

            if (ctx->param && !is_ellipsis(ctx->param->sym)) {
                const struct position *pos = node->r ? &node->r->pos : &node->pos;
                add_error(ctx->diag, pos, "too few arguments to function call");
            }

            ctx->param = tmp;
            return;
        }

    case NOD_ARG:
        if (!ctx->param) {
            add_error(ctx->diag, &node->pos, "too many arguments to function call");
            return;
        }

        if (!is_compatible(node->type, ctx->param->sym->type) &&
            !is_ellipsis(ctx->param->sym)) {
            make_type_name(node->type, type_name1);
            make_type_name(ctx->param->sym->type, type_name2);
            add_error(ctx->diag, &node->pos,
                    "passing '%s' to parameter of incompatible type '%s'",
                    type_name1, type_name2);
        }
        ctx->param = next_param_(ctx->param);

        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        return;

    /* break and continue */
    case NOD_BREAK:
        if (ctx->loop_depth == 0 && ctx->switch_depth == 0)
            add_error(ctx->diag, &node->pos,
                    "'break' statement not in loop or switch statement");
        break;

    case NOD_CONTINUE:
        if (ctx->loop_depth == 0)
            add_error(ctx->diag, &node->pos,
                    "'continue' statement not in loop statement");
        break;

    case NOD_RETURN:
        {
            const struct data_type *ret_type = return_type_(ctx->func_type);
            const char *func_name = ctx->func_type->sym->name;

            if (!is_void(ret_type) && !node->l) {
                add_error(ctx->diag, &node->pos,
                        "non-void function '%s' should return a value", func_name);
            }
            else if (is_void(ret_type) && node->l) {
                add_error(ctx->diag, &node->l->pos,
                        "void function '%s' should not return a value", func_name);
            }
            else if (node->l && !is_compatible(ret_type, node->l->type)) {
                make_type_name(node->l->type, type_name1);
                make_type_name(ret_type, type_name2);
                add_error(ctx->diag, &node->l->pos,
                        "returning '%s' from a function with incompatible result type '%s'",
                        type_name1, type_name2);
            }
        }
        break;

    default:
        break;;
    }

    check_tree_(node->l, ctx);
    check_tree_(node->r, ctx);
}

static void check_tree_semantics(struct ast_node *tree, struct diagnostic *diag)
{
    struct tree_context ctx = {0};
    ctx.diag = diag;
    check_tree_(tree, &ctx);
}

int analyze_semantics(struct ast_node *tree,
        struct symbol_table *table, struct diagnostic *diag)
{
    check_tree_semantics(tree, diag);
    check_symbol_usage(table, diag);

    return 0;
}
