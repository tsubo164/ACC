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
        return n2->dtype;
    }

    if (!n2) {
        return n1->dtype;
    }

    if (n1->dtype->kind > n2->dtype->kind) {
        return n1->dtype;
    } else {
        return n2->dtype;
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
        /* TODO may need SYM_MEMBER */
        if (sym->kind == SYM_VAR && sym->scope_level > 0) {
            const int size  = sym->dtype->byte_size;
            const int align = sym->dtype->alignment;
            const int len   = sym->dtype->array_len;

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

    strc->dtype->byte_size = align_to(total_offset, strc->dtype->alignment);
    strc->mem_offset = strc->dtype->byte_size;
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

            if (sym->dtype->kind == DATA_TYPE_STRUCT) {
                struct symbol *strc = lookup_symbol(table, sym->dtype->tag, SYM_STRUCT);
                size  = strc->dtype->byte_size;
                align = strc->dtype->alignment;
                len   = strc->dtype->array_len;
            } else {
                size  = sym->dtype->byte_size;
                align = sym->dtype->alignment;
                len   = sym->dtype->array_len;
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

        if (symbol_flag_is_on(sym, IS_REDEFINED)) {
            add_error(messages, "redefinition of variable", sym->file_pos);
        }
    }
    return 0;
}

enum decl_kind {
    DECL_VAR,
    DECL_FUNC,
    DECL_PARAM,
    DECL_STRUCT
};

struct declaration {
    int kind;
    const char *ident;
    struct data_type *type;
    int has_func_def;
    int has_member_decl;
};

static int sym_kind_of(const struct declaration *decl)
{
    switch (decl->kind) {
    case DECL_VAR:    return SYM_VAR;
    case DECL_FUNC:   return SYM_FUNC;
    case DECL_PARAM:  return SYM_PARAM;
    case DECL_STRUCT: return SYM_STRUCT;
    default:         return SYM_VAR;
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
            /* TODO may need DECL_MEMBER */
            new_decl.kind = DECL_VAR;
            add_sym_(tree->l, table, &new_decl);
            add_sym_(tree->r, table, &new_decl);
            return;
        }

    case NOD_DECL_INIT:
        {
            struct declaration new_decl = {0};
            duplicate_decl(&new_decl, decl);
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
                    sym->dtype = decl->type;
                    tree->data.sym = sym;
                }
                else {
                    struct symbol *sym = NULL;
                    sym = use_symbol(table, decl->ident, SYM_STRUCT);
                    decl->type = sym->dtype;
                    tree->data.sym = sym;
                }
            } else {
                struct symbol *sym = NULL;
                sym = define_symbol(table, decl->ident, sym_kind_of(decl));
                sym->dtype = decl->type;
                tree->data.sym = sym;
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

            sym_l = tree->l->data.sym;

            /* TODO we can not use node->dtype because dtype is not set yet
             * when adding symbol
             * should define sym_of(node) or symbol_(node)
             * text_(node), int_(node) * set_text(node, ...), set_int(node, ...)
             * L_(node), R_(node)
             */
            tag = sym_l->dtype->tag;
            sym = lookup_symbol(table, tag, SYM_STRUCT);

            for (;;) {
                if (sym->kind == SYM_SCOPE_END) {
                    /* end of struct definition */
                    break;
                }

                if (sym->name && !strcmp(sym->name, mem)) {
                    tree->r->data.sym = sym;
                    /* TODO adding dtype to node? */
                    tree->r->dtype = sym->dtype;
                    tree->data.sym = sym;
                    tree->dtype = sym->dtype;
                    break;
                }
                sym++;
            }
        }
        return;

    case NOD_IDENT:
        {
            struct symbol *sym = NULL;
            int sym_kind = SYM_VAR;

            /* TODO The parameter 'sim_kind' may not be needed.
             * The symbol knows what kind of symbol it is.  */
            sym = use_symbol(table, tree->sval, sym_kind);
            tree->data.sym = sym;
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
        decl->type = type_array(decl->type, tree->data.ival);
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
        node->dtype = node->l->dtype;
        break;

    case NOD_DEREF:
        node->dtype = promote_data_type(node->l, node->r);
        node->dtype = node->dtype->ptr_to;;
        break;

    case NOD_CALL:
        node->dtype = node->l->dtype;
        break;

    case NOD_STRUCT_REF:
        node->dtype = node->data.sym->dtype;
        break;

    /* nodes with symbol */
    case NOD_DECL_IDENT:
    case NOD_IDENT:
        node->dtype = node->data.sym->dtype;
        break;

    /* nodes with literal */
    case NOD_NUM:
        node->dtype = type_int();
        break;

    default:
        node->dtype = promote_data_type(node->l, node->r);
        break;
    }
}

int semantic_analysis(struct ast_node *tree,
        struct symbol_table *table, struct message_list *messages)
{
    add_symbols(tree, table);

    if (0)
        analyze_symbol_usage(table, messages);

    add_types(tree, table);
    allocate_local_storage(table);

    return 0;
}
