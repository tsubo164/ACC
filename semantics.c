#include <stdio.h>
#include <string.h>
#include "semantics.h"
#include "message.h"
#include "ast.h"

static int check_symbol_usage(struct symbol_table *table, struct message_list *messages)
{
    struct symbol *sym;

    for (sym = table->head; sym; sym = sym->next) {
        /* TODO remove this */
        const struct position pos = {0};

        if (is_local_var(sym)) {
            /* TODO support check for struct variables */
            if (is_struct(sym->type))
                continue;

            if (sym->is_redefined)
                add_error2(messages, &sym->pos, "redefinition of '%s'", sym->name);

            if (sym->is_defined && !sym->is_used)
                add_warning2(messages, &sym->pos, "unused variable '%s'", sym->name);
        }
        else if (is_global_var(sym)) {
            if (is_static(sym) && !sym->is_used)
                add_warning2(messages, &sym->pos, "unused variable '%s'", sym->name);
        }
        else if (is_enumerator(sym)) {
            if (sym->is_assigned)
                add_error(messages, "expression is not assignable", &pos);
        }
        else if (is_func(sym)) {
            if (sym->is_defined && !sym->is_used && is_static(sym))
                add_warning2(messages, &sym->pos, "unused function '%s'", sym->name);

            /* TODO consider removing this by checking all symbols in tree nodes */
            if (!sym->is_defined && sym->is_used && !is_extern(sym))
                add_warning2(messages, &sym->pos,
                        "implicit declaration of function '%s'", sym->name);
        }
        else if (is_case(sym)) {
            if (sym->is_redefined)
                add_error2(messages, &sym->pos, "duplicate case value '%d'", sym->mem_offset);
        }
        else if (is_default(sym)) {
            if (sym->is_redefined)
                add_error2(messages, &sym->pos, "multiple default labels in one switch");
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
    const struct symbol *func_sym;
    const struct symbol *param_sym;
    int loop_depth;
    int switch_depth;
    int is_lvalue;
    int has_init;

    /* for initializers */
    const struct symbol *struct_sym;
    int array_length;
    int index;
};

static void check_initializer(struct ast_node *node, struct tree_context *ctx);

static int is_null_pointer(struct ast_node *node)
{
    if (is_integer(node->type) &&
        node->kind == NOD_NUM &&
        node->ival == 0)
        return 1;
    return 0;
}

static void check_init_scalar(struct ast_node *node, struct tree_context *ctx)
{
    struct data_type *t1, *t2;

    if (!node)
        return;

    switch (node->kind) {

    case NOD_INIT:
        t1 = node->type;
        t2 = node->r->type;

        /* zero to null pointer */
        if (is_pointer(node->type) && is_null_pointer(node->r))
            return;

        if (!is_compatible(t1, t2)) {
            if (!is_array(t1) && !is_struct(t1))
                add_error2(ctx->messages, &node->pos,
                        "initializing '%s' with an expression of incompatible type '%s'",
                        type_name_of(t1), type_name_of(t2));
        }
        break;

    case NOD_DECL_INIT:
        check_init_scalar(node->r, ctx);
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
        if (ctx->index >= ctx->array_length) {
            add_error2(ctx->messages, &node->pos,
                    "excess elements in array initializer");
            break;
        }

        check_initializer(node, ctx);
        ctx->index++;
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
        if (!ctx->struct_sym) {
            add_error2(ctx->messages, &node->pos,
                    "excess elements in struct initializer");
            break;
        }

        check_initializer(node, ctx);
        ctx->struct_sym = next_member(ctx->struct_sym);
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
    if (!node || !node->r)
        return;

    if (is_array(node->type)) {
        struct tree_context new_ctx = *ctx;

        new_ctx.index = 0;
        new_ctx.array_length = get_array_length(node->type);

        check_init_array_element(node->r, &new_ctx);
    }
    else if(is_struct(node->type)) {
        struct tree_context new_ctx = *ctx;

        new_ctx.index = 0;
        new_ctx.struct_sym = first_member(symbol_of(node->type));

        check_init_struct_members(node->r, &new_ctx);
    }
    else {
        check_init_scalar(node, ctx);
    }
}

static void check_init_(struct ast_node *node, struct tree_context *ctx)
{
    if (!node)
        return;

    switch (node->kind) {

    case NOD_DECL_INIT:
        check_init_(node->l, ctx);
        check_init_(node->r, ctx);

        ctx->index = 0;
        ctx->struct_sym = node->l->type->sym;
        check_initializer(node, ctx);
        return;

    default:
        break;;
    }

    check_init_(node->l, ctx);
    check_init_(node->r, ctx);
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
        if (is_func(node->sym))
            ctx->func_sym = node->sym;

        if (is_incomplete(node->sym->type) &&
                (is_local_var(node->sym) || is_global_var(node->sym))) {
            add_error2(ctx->messages, &node->pos, "variable has incomplete type '%s'",
                    type_name_of(node->sym->type));
            return;
        }
        break;

    /* TODO add sub_assign, ... */
    case NOD_ASSIGN:
        /* evaluate rvalue first to check a = a + 1; */
        check_tree_(node->r, ctx);
        ctx->is_lvalue = 1;
        check_tree_(node->l, ctx);
        ctx->is_lvalue = 0;

        {
            /* TODO not work for array lvalue. need to find ident */
            struct symbol *sym = node->l->sym;
            if (is_const(node->type))
                add_error2(ctx->messages, &node->pos,
                        "cannot assign to variable '%s' with const-qualified", sym->name);
        }
        return;

    case NOD_IDENT:
        if (ctx->is_lvalue && !node->sym->is_used)
            node->sym->is_initialized = 1;
        node->sym->is_assigned = ctx->is_lvalue;
        node->sym->is_used = 1;

        {
            struct symbol *sym = node->sym;

            if (is_local_var(sym)) {
                if (sym->is_defined && sym->is_used && !sym->is_initialized)
                    /* array, struct, union will not be treated as uninitialized */
                    if (!is_array(sym->type) && !is_struct(sym->type))
                        add_warning2(ctx->messages, &node->pos,
                                "variable '%s' is uninitialized when used here", sym->name);

                if (!sym->is_defined && sym->is_used)
                    add_error2(ctx->messages, &node->pos,
                            "use of undeclared identifier '%s'", sym->name);
            }
        }
        break;

    /* struct */
    case NOD_STRUCT_REF:
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        if (!is_struct(node->l->type)) {
            add_error2(ctx->messages, &node->pos,
                    "member reference base type '%.32s' is not a structure or union",
                    type_name_of(node->l->type));
            return;
        }
        if (is_incomplete(node->l->type)) {
            add_error2(ctx->messages, &pos,
                    "incomplete definition of type 'struct %.32s'", tag_of(node->l->type));
            return;
        }
        if (!node->r->sym->is_defined)
            add_error2(ctx->messages, &node->pos,
                    "no member named '%.32s' in 'struct %.32s'",
                    node->r->sym->name, tag_of(node->l->type));
        return;

    case NOD_DEREF:
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        if (!underlying(node->l->type))
            add_error(ctx->messages, "indirection requires pointer operand", &node->pos);
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
    case NOD_DECL_FUNC:
        ctx->func_sym = NULL;
        break;

    case NOD_FUNC_DEF:
        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
        ctx->func_sym = NULL;
        return;

    case NOD_CALL:
        {
            const struct symbol *tmp = ctx->param_sym;

            check_tree_(node->l, ctx);
            ctx->param_sym = first_param(node->l->sym);
            check_tree_(node->r, ctx);

            if (ctx->param_sym && !is_ellipsis(ctx->param_sym)) {
                const struct position *pos = node->r ? &node->r->pos : &node->pos;
                add_error2(ctx->messages, pos, "too few arguments to function call");
            }

            ctx->param_sym = tmp;
            return;
        }

    case NOD_ARG:
        if (!ctx->param_sym) {
            add_error2(ctx->messages, &node->pos, "too many arguments to function call");
            return;
        }

        if (!is_compatible(node->type, ctx->param_sym->type) &&
            !is_ellipsis(ctx->param_sym)) {
            add_error2(ctx->messages, &node->pos,
                    "incompatible conversion from '%s' to '%s'",
                    type_name_of(node->type), type_name_of(ctx->param_sym->type));
        }
        ctx->param_sym = next_param(ctx->param_sym);

        check_tree_(node->l, ctx);
        check_tree_(node->r, ctx);
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
        if (!is_void(ctx->func_sym->type) && !node->l)
            add_error2(ctx->messages, &node->pos,
                    "non-void function '%s' should return a value", ctx->func_sym->name);
        else if (is_void(ctx->func_sym->type) && node->l)
            add_error2(ctx->messages, &node->l->pos,
                    "void function '%s' should not return a value", ctx->func_sym->name);

        else if (node->l && !is_compatible(ctx->func_sym->type, node->l->type))
            add_error2(ctx->messages, &node->l->pos,
                    "incompatible conversion from '%s' to '%s'",
                    type_name_of(node->l->type), type_name_of(ctx->func_sym->type));
        break;

    default:
        break;;
    }

    check_tree_(node->l, ctx);
    check_tree_(node->r, ctx);
}

static void check_initializer_semantics(struct ast_node *tree, struct message_list *messages)
{
    struct tree_context ctx = {0};
    ctx.messages = messages;
    check_init_(tree, &ctx);
}

static void check_tree_semantics(struct ast_node *tree, struct message_list *messages)
{
    struct tree_context ctx = {0};
    ctx.messages = messages;
    check_tree_(tree, &ctx);
}

int semantic_analysis(struct ast_node *tree,
        struct symbol_table *table, struct message_list *messages)
{
    check_tree_semantics(tree, messages);
    check_symbol_usage(table, messages);

    /* needs to be called after type sizes are solved */
    check_initializer_semantics(tree, messages);

    return 0;
}
