#include <stdio.h>
#include <string.h>
#include "semantics.h"
#include "message.h"

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

static void scope_begin(struct symbol_table *table)
{
    symbol_scope_begin(table);
}

static void scope_end(struct symbol_table *table)
{
    symbol_scope_end(table);
}

static int align_to(int pos, int align)
{
    return ((pos + align - 1) / align) * align;
}

static void compute_struct_size(struct symbol_table *table, struct symbol *strc)
{
    struct symbol *sym = strc;
    int total_offset = 0;

    for (;;) {
        /* TODO support nested struct by checking scope level */
        if (sym->kind == SYM_MEMBER) {
            const int size  = sym->type->byte_size;
            const int align = sym->type->alignment;
            const int len   = sym->type->array_len;

            total_offset = align_to(total_offset, align);
            sym->mem_offset = total_offset;
            total_offset += len * size;
        }

        /* TODO struct scope_level is NOT ALWAYS 1 */
        if (sym->kind == SYM_SCOPE_END && sym->scope_level == 1) {
            break;
        }
        sym++;
    }

    strc->type->byte_size = align_to(total_offset, strc->type->alignment);
    strc->mem_offset = strc->type->byte_size;
}

static void compute_func_stack_size(struct symbol_table *table, struct symbol *func)
{
    struct symbol *sym = func;
    int total_offset = 0;

    for (;;) {
        if (is_param(sym) || is_local_var(sym)) {
            int size  = 0;
            int align = 0;
            int len   = 0;

            if (sym->type->kind == DATA_TYPE_STRUCT) {
                struct symbol *strc = lookup_symbol(table, sym->type->tag, SYM_STRUCT);
                size  = strc->type->byte_size;
                align = strc->type->alignment;
                len   = strc->type->array_len;
            } else {
                size  = sym->type->byte_size;
                align = sym->type->alignment;
                len   = sym->type->array_len;
            }

            total_offset = align_to(total_offset, align);
            total_offset += len * size;
            sym->mem_offset = total_offset;
        }

        if (sym->kind == SYM_STRUCT) {
            compute_struct_size(table, sym);
        }

        if (sym->kind == SYM_SCOPE_END && sym->scope_level == 1) {
            break;
        }
        sym++;
    }

    func->mem_offset = align_to(total_offset, 16);
}

static void allocate_local_storage(struct symbol_table *table)
{
    const int N = get_symbol_count(table);
    int i;

    for (i = 0; i < N; i++) {
        struct symbol *sym = get_symbol(table, i);

        if (sym->kind == SYM_FUNC) {
            compute_func_stack_size(table, sym);
        }

        if (sym->kind == SYM_STRUCT) {
            compute_struct_size(table, sym);
        }
    }
}

static int analyze_symbol_usage(struct symbol_table *table, struct message_list *messages)
{
    const int N = get_symbol_count(table);
    int i;

    for (i = 0; i < N; i++) {
        const struct symbol *sym = get_symbol(table, i);

        if (is_local_var(sym)) {
            if (sym->is_redefined)
                add_error(messages, "redefinition of variable", sym->file_pos);

            if (!sym->is_defined && sym->is_used)
                add_error(messages, "use of undefined symbol", sym->file_pos);

            if (sym->is_defined && !sym->is_used)
                add_warning(messages, "unused variable", sym->file_pos);

            /*
            if (sym->is_defined && sym->is_used && !sym->is_initialized)
                add_warning(messages, "uninitialized variable used", sym->file_pos);
            */
        }
        else if (is_func(sym)) {
            if (!sym->is_defined && sym->is_used)
                add_warning(messages, "implicit declaration of function", sym->file_pos);
        }
    }
    return 0;
}

enum decl_kind {
    DECL_VAR,
    DECL_FUNC,
    DECL_PARAM,
    DECL_STRUCT,
    DECL_MEMBER
};

struct declaration {
    int kind;
    const char *ident;
    struct data_type *type;
    int has_func_def;
    int has_member_decl;
    int has_init;
    int is_func_call;
    int is_lvalue;
};

static int sym_kind_of(const struct declaration *decl)
{
    switch (decl->kind) {
    case DECL_VAR:    return SYM_VAR;
    case DECL_FUNC:   return SYM_FUNC;
    case DECL_PARAM:  return SYM_PARAM;
    case DECL_STRUCT: return SYM_STRUCT;
    case DECL_MEMBER: return SYM_MEMBER;
    default:          return SYM_VAR;
    }
}

static void duplicate_decl(struct declaration *dest, const struct declaration *src)
{

    /* TODO make duplicate_type in data_type.c */
    *dest = *src;
}

static void add_sym_(struct ast_node *tree,
        struct symbol_table *table, struct declaration *decl)
{
    if (!tree)
        return;

    switch (tree->kind) {

    case NOD_DECL:
        {
            struct declaration new_decl = {0};
            if (decl->has_func_def)
                new_decl.has_func_def = 1;
            add_sym_(tree->l, table, &new_decl);
            /* TODO DECL_* may have nothing to with SYM_*.
             * e.g. type spec declared as struct(SYM_STRUCT) and
             * identifier declared as a variable (SYM_VAR) */
            new_decl.kind = DECL_VAR;
            add_sym_(tree->r, table, &new_decl);
            return;
        }

    case NOD_DECL_PARAM:
        {
            struct declaration new_decl = {0};
            new_decl.kind = DECL_PARAM;
            add_sym_(tree->l, table, &new_decl);
            add_sym_(tree->r, table, &new_decl);
            return;
        }

    case NOD_DECL_MEMBER:
        {
            struct declaration new_decl = {0};
            new_decl.kind = DECL_MEMBER;
            add_sym_(tree->l, table, &new_decl);
            add_sym_(tree->r, table, &new_decl);
            return;
        }

    case NOD_DECL_INIT:
        {
            struct declaration new_decl = {0};
            duplicate_decl(&new_decl, decl);
            new_decl.has_init = tree->r ? 1 : 0;
            add_sym_(tree->l, table, &new_decl);
            add_sym_(tree->r, table, &new_decl);
            return;
        }

    case NOD_DECL_IDENT:
        decl->ident = tree->sval;
        {
            if (decl->kind == DECL_STRUCT) {
                if (decl->has_member_decl) {
                    struct symbol *sym = NULL;
                    sym = define_symbol(table, decl->ident, SYM_STRUCT);
                    decl->type = type_struct(decl->ident);
                    /* TODO need to pass type to define_symbol */
                    sym->type = decl->type;
                    tree->sym = sym;
                }
                else {
                    struct symbol *sym = NULL;
                    sym = use_symbol(table, decl->ident, SYM_STRUCT);
                    decl->type = sym->type;
                    tree->sym = sym;
                }
            } else {
                struct symbol *sym = NULL;
                sym = define_symbol(table, decl->ident, sym_kind_of(decl));
                /* TODO need to pass this info to define_symbol */
                sym->is_initialized = decl->has_init;
                /* TODO need to pass type to define_symbol */
                sym->type = decl->type;
                tree->sym = sym;
            }
            return;
        }

    case NOD_STRUCT_REF:
        {
            const struct symbol *sym_l;
            const struct symbol *sym;
            const char *mem = tree->r->sval;
            const char *tag;

            add_sym_(tree->l, table, decl);

            sym_l = tree->l->sym;

            /* TODO we can not use node->type because type is not set yet
             * when adding symbol
             * should define sym_of(node) or symbol_(node)
             * text_(node), int_(node) * set_text(node, ...), set_int(node, ...)
             * L_(node), R_(node)
             */
            tag = sym_l->type->tag;
            sym = lookup_symbol(table, tag, SYM_STRUCT);

            for (;;) {
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
                sym++;
            }
        }
        return;

    case NOD_IDENT:
        {
            struct symbol *sym = NULL;
            const int sym_kind = decl->is_func_call ? SYM_FUNC : SYM_VAR;

            if (decl->is_lvalue) {
                sym = assign_to_symbol(table, tree->sval, sym_kind);
                decl->is_lvalue = 0;
            } else {
                sym = use_symbol(table, tree->sval, sym_kind);
            }

            tree->sym = sym;
            return;
        }

    case NOD_CALL:
        {
            struct declaration new_decl = {0};
            new_decl.is_func_call = 1;
            add_sym_(tree->l, table, &new_decl);
            add_sym_(tree->r, table, decl);
            return;
        }

    case NOD_ASSIGN:
        {
            struct declaration new_decl = {0};
            new_decl.is_lvalue = 1;
            add_sym_(tree->l, table, &new_decl);
            add_sym_(tree->r, table, decl);
            return;
        }

    case NOD_DECL_FUNC:
        decl->kind = DECL_FUNC;
        add_sym_(tree->l, table, decl);
        scope_begin(table);
        add_sym_(tree->r, table, decl);
        if (!decl->has_func_def)
            scope_end(table);
        return;

    case NOD_FUNC_DEF:
        decl->has_func_def = 1;
        add_sym_(tree->l, table, decl);
        add_sym_(tree->r, table, decl);
        scope_end(table);
        return;

    case NOD_COMPOUND:
        scope_begin(table);
        add_sym_(tree->l, table, decl);
        add_sym_(tree->r, table, decl);
        scope_end(table);
        return;

    case NOD_SPEC_STRUCT:
        decl->kind = DECL_STRUCT;
        if (tree->r)
            decl->has_member_decl = 1;
        add_sym_(tree->l, table, decl);

        if (tree->r) {
            scope_begin(table);
            add_sym_(tree->r, table, decl);
            scope_end(table);
        }
        return;

    case NOD_SPEC_ARRAY:
        decl->type = type_array(decl->type, tree->ival);
        break;

    case NOD_SPEC_POINTER:
        decl->type = type_ptr(decl->type);
        break;

    case NOD_SPEC_CHAR:
        decl->type = type_char();
        break;

    case NOD_SPEC_INT:
        decl->type = type_int();
        break;

    default:
        break;
    }

    add_sym_(tree->l, table, decl);
    add_sym_(tree->r, table, decl);
}

static void add_symbols(struct ast_node *tree, struct symbol_table *table)
{
    struct declaration decl = {0};
    add_sym_(tree, table, &decl);
    return;
}

static void add_types(struct ast_node *node, struct symbol_table *table)
{
    if (!node)
        return;

    add_types(node->l, table);
    add_types(node->r, table);

    switch (node->kind) {

    case NOD_ASSIGN:
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

    default:
        node->type = promote_data_type(node->l, node->r);
        break;
    }
}

int semantic_analysis(struct ast_node *tree,
        struct symbol_table *table, struct message_list *messages)
{
    if (0) {
    add_symbols(tree, table);
    }

    analyze_symbol_usage(table, messages);

    add_types(tree, table);
    allocate_local_storage(table);

    return 0;
}
