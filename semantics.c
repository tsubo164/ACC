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

static int align_to(int pos, int align)
{
    return ((pos + align - 1) / align) * align;
}

static void compute_type_name_size(struct symbol_table *table, struct symbol *type_name)
{
    type_name->mem_offset = get_size(type_name->type);
}

static void compute_enum_size(struct symbol_table *table, struct symbol *enm)
{
    enm->mem_offset = get_size(enm->type);
}

/* TODO this might be good to go to symbol.c */
static void compute_struct_size(struct symbol_table *table, struct symbol *strc)
{
    struct symbol *sym;
    int total_offset = 0;
    int struct_size = 0;
    /* inside of struct is one level upper than struct scope */
    const int struct_scope = strc->scope_level + 1;

    /* incomplete struct type */
    if (!strc->is_defined)
        return;

    for (sym = strc; sym; sym = sym->next) {
        if (sym->kind == SYM_MEMBER) {
            const int size  = get_size(sym->type);
            const int align = get_alignment(sym->type);

            total_offset = align_to(total_offset, align);
            sym->mem_offset = total_offset;
            total_offset += size;
        }

        if (sym->kind == SYM_SCOPE_END && sym->scope_level == struct_scope)
            break;
    }

    struct_size = align_to(total_offset, get_alignment(strc->type));

    set_struct_size(strc->type, struct_size);
    strc->mem_offset = struct_size;
}

static void compute_func_size(struct symbol_table *table, struct symbol *func)
{
    struct symbol *sym;
    int total_offset = 0;

    for (sym = func; sym; sym = sym->next) {
        if (is_param(sym) || is_local_var(sym)) {
            const int size  = get_size(sym->type);
            const int align = get_alignment(sym->type);

            total_offset = align_to(total_offset, align);
            total_offset += size;
            sym->mem_offset = total_offset;
        }

        if (sym->kind == SYM_TAG_STRUCT)
            compute_struct_size(table, sym);

        if (sym->kind == SYM_TAG_ENUM)
            compute_enum_size(table, sym);

        if (sym->kind == SYM_TYPEDEF)
            compute_type_name_size(table, sym);

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

        if (sym->kind == SYM_TYPEDEF)
            compute_type_name_size(table, sym);
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
            if (is_struct(sym->type))
                continue;

            if (sym->is_redefined)
                add_error(messages, "redefinition of variable", &pos);

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
    const struct symbol *func_sym;
    const struct symbol *var_sym;
    int loop_depth;
    int switch_depth;
    int enum_value;
    int is_lvalue;
    int has_init;

    const struct symbol *struct_sym;
    int index;
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

static void check_init_(struct ast_node *node, struct tree_context *ctx,
        struct data_type *type)
{
    static int index = 0;

    if (!node)
        return;

    switch (node->kind) {

    case NOD_LIST:
        check_init_(node->l, ctx, type);
        node->ival = index;

        if (is_array(type)) {
            check_init_(node->r, ctx, type);
            node->r->ival = index;
        }
        index++;

        if (index > get_array_length(ctx->var_sym->type) &&
            has_unkown_array_length(type))
            add_error2(ctx->messages, &node->pos,
                    "excess elements in array initializer");

        if (!is_compatible(type, node->r->type))
            if (!is_array(type))
                add_error2(ctx->messages, &node->pos,
                        "initializing '%s' with an expression of incompatible type '%s'",
                        type_name_of(type), type_name_of(node->r->type));
        break;

    case NOD_INIT_LIST:
        {
            const int tmp = index;
            index = 0;

            check_init_(node->l, ctx, underlying(type));
            if (has_unkown_array_length(type))
                set_array_length(type, index);

            index = tmp;
        }
        break;

    default:
        break;
    }
}

static void check_init_struct_(struct ast_node *node, struct tree_context *ctx)
{
    if (!node)
        return;

    switch (node->kind) {

    case NOD_LIST:
        check_init_struct_(node->l, ctx);
        check_init_struct_(node->r, ctx);

        if (ctx->struct_sym && ctx->struct_sym->kind != SYM_MEMBER)
            add_error2(ctx->messages, &node->pos,
                    "excess elements in struct initializer");

        if (!is_compatible(ctx->struct_sym->type, node->r->type))
            if (!is_struct(ctx->struct_sym->type))
                add_error2(ctx->messages, &node->pos,
                        "initializing '%s' with an expression of incompatible type '%s'",
                        type_name_of(ctx->struct_sym->type), type_name_of(node->r->type));

        node->ival = ctx->index++;
        ctx->struct_sym = ctx->struct_sym->next;
        break;

    case NOD_INIT_LIST:
        {
            const struct tree_context tmp = *ctx;

            ctx->struct_sym = ctx->struct_sym->next->next;
            ctx->index = 0;
            check_init_struct_(node->l, ctx);

            *ctx = tmp;
        }
        break;

    default:
        break;
    }
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
        ctx->var_sym = NULL;

        ctx->has_init = node->r ? 1 : 0;
        check_tree_(node->l, ctx);
        ctx->has_init = 0;
        check_tree_(node->r, ctx);

        if (!is_struct(node->type)) {
            check_init_(node->r, ctx, node->type);
        } else {
            ctx->struct_sym = node->type->sym;
            ctx->index = 0;
            check_init_struct_(node->r, ctx);
        }

        ctx->var_sym = NULL;
        return;

    case NOD_DECL_IDENT:
        node->sym->is_initialized = ctx->has_init;
        if (node->sym->kind == SYM_FUNC)
            ctx->func_sym = node->sym;
        if (node->sym->kind == SYM_VAR)
            ctx->var_sym = node->sym;

        if (is_incomplete(node->sym->type) &&
                (is_local_var(node->sym) || is_global_var(node->sym))) {
            add_error(ctx->messages, "variable has incomplete type ''", &pos);
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
                                "uninitialized variable '%s'", sym->name);

                if (!sym->is_defined && sym->is_used)
                    add_error2(ctx->messages, &node->pos,
                            "undeclared identifier '%s'", sym->name);
            }
        }
        break;

    /* array */
    case NOD_SPEC_ARRAY:
        check_tree_(node->l, ctx);
        if (node->l)
            set_array_length(node->type, node->l->ival);
        check_tree_(node->r, ctx);
        return;

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
        if (is_void(ctx->func_sym->type) && node->l)
            add_error(ctx->messages, "function '' should not return a value", &pos);
        if (!is_void(ctx->func_sym->type) && !node->l)
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

int semantic_analysis(struct ast_node *tree,
        struct symbol_table *table, struct message_list *messages)
{
    check_tree_semantics(tree, messages);
    check_symbol_usage(table, messages);

    add_symbol_size(table);

    return 0;
}
