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

static struct data_type *make_type(const struct ast_node *node)
{
    if (!node)
        return NULL;

    switch (node->kind) {

    case NOD_TYPE_CHAR:
        return type_char();

    case NOD_TYPE_INT:
        return type_int();

    case NOD_TYPE_POINTER:
        return type_ptr(make_type(node->l));

    case NOD_TYPE_ARRAY:
        return type_array(make_type(node->l), node->data.ival);

    case NOD_TYPE_STRUCT:
        return type_struct(node->sval);

    default:
        return NULL;
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

static void define_sym(struct ast_node *node, struct symbol_table *table, int sym_kind)
{
    const char *name = node->sval;
    struct symbol *sym = define_symbol(table, name, sym_kind);

    sym->dtype = make_type(node->l);
    node->data.sym = sym;
}

static void use_sym(struct ast_node *node, struct symbol_table *table, int sym_kind)
{
    const char *name = node->sval;
    struct symbol *sym = use_symbol(table, name, sym_kind);

    node->data.sym = sym;

    /* TODO find better way */
    if (is_param(sym)) {
        node->kind = NOD_PARAM;
    } else if (is_global_var(sym)) {
        node->kind = NOD_GLOBAL_VAR;
    }
}

static void add_symbol(struct ast_node *node, struct symbol_table *table)
{
    if (!node)
        return;

    switch (node->kind) {

    /* TODO we don't know the 'type of var' at this point
     * they all come as NOD_VAR from parser
     */
    /*
    case NOD_GLOBAL_VAR:
    case NOD_PARAM:
    */
    case NOD_VAR:
        use_sym(node, table, SYM_VAR);
        add_symbol(node->l, table);
        add_symbol(node->r, table);
        break;

    case NOD_PARAM_DEF:
        define_sym(node, table, SYM_PARAM);
        add_symbol(node->l, table);
        add_symbol(node->r, table);
        break;

    case NOD_CALL:
        use_sym(node, table, SYM_FUNC);
        add_symbol(node->l, table);
        add_symbol(node->r, table);
        break;

    case NOD_VAR_DEF:
        define_sym(node, table, SYM_VAR);
        add_symbol(node->l, table);
        add_symbol(node->r, table);
        break;

    case NOD_FUNC_DEF:
        define_sym(node, table, SYM_FUNC);
        scope_begin(table);
        add_symbol(node->l, table);
        add_symbol(node->r, table);
        scope_end(table);
        break;

    case NOD_STRUCT_REF:
        {
            const struct symbol *sym;
            const char *mem = node->r->sval;
            const char *tag;

            add_symbol(node->l, table);
            /* TODO we can not use node->dtype because dtype is not set yet
             * when adding symbol
             * should define sym_of(node) or symbol_(node)
             * text_(node), int_(node) * set_text(node, ...), set_int(node, ...)
             * L_(node), R_(node)
             */
            tag = node->l->data.sym->dtype->tag;

            sym = lookup_symbol(table, tag, SYM_STRUCT);

            for (;;) {
                if (sym->kind == SYM_SCOPE_END) {
                    /* end of struct definition */
                    break;
                }

                if (sym->name && !strcmp(sym->name, mem)) {
                    node->r->data.sym = sym;
                    node->r->dtype = sym->dtype;
                    break;
                }
                sym++;
            }
        }
        break;

    case NOD_STRUCT_DECL:
        define_sym(node, table, SYM_STRUCT);
        scope_begin(table);
        add_symbol(node->l, table);
        add_symbol(node->r, table);
        scope_end(table);
        break;

    case NOD_COMPOUND:
        scope_begin(table);
        add_symbol(node->l, table);
        add_symbol(node->r, table);
        scope_end(table);
        break;

    default:
        add_symbol(node->l, table);
        add_symbol(node->r, table);
        break;
    }
}

static int add_type(struct ast_node *node, struct symbol_table *table)
{
    if (!node) {
        return 0;
    }

    add_type(node->l, table);
    add_type(node->r, table);

    /* promote */
    switch (node->kind) {

    case NOD_ADD:
        node->dtype = promote_data_type(node->l, node->r);

        if (node->l->dtype->kind == DATA_TYPE_ARRAY) {
            struct ast_node *size, *mul;

            size = new_ast_node(NOD_NUM, NULL, NULL);
            size->data.ival = node->l->dtype->ptr_to->byte_size;
            mul = new_ast_node(NOD_MUL, size, node->r);
            node->r = mul;
        }
        break;

    case NOD_SUB:
    case NOD_MUL:
    case NOD_DIV:
        /*
    case NOD_ASSIGN:
    case NOD_EQ:
        */
        node->dtype = promote_data_type(node->l, node->r);
        break;

    case NOD_ASSIGN:
        node->dtype = node->l->dtype;
        break;

    case NOD_DEREF:
        node->dtype = promote_data_type(node->l, node->r);
        node->dtype = node->dtype->ptr_to;;
        break;

    case NOD_STRUCT_REF:
        {
            const struct symbol *sym;
            const char *mem = node->r->sval;

            sym = lookup_symbol(table, node->l->data.sym->dtype->tag, SYM_STRUCT);
            for (;;) {
                if (sym->kind == SYM_SCOPE_END) {
                    /* end of struct definition */
                    break;
                }

                if (sym->name && !strcmp(sym->name, mem)) {
                    node->r->data.sym = sym;
                    node->dtype = node->r->dtype;

                    /* struct var data type <- member data type */
                    node->dtype = sym->dtype;
                    node->r->dtype = sym->dtype;

                    break;
                }
                sym++;
            }
        }
        break;

        /* nodes with symbol */
    case NOD_VAR:
    case NOD_VAR_DEF:
    case NOD_PARAM_DEF:
    case NOD_CALL:
        node->dtype = node->data.sym->dtype;
        break;

        /* nodes with literal */
    case NOD_NUM:
        node->dtype = type_int();
        break;

    default:
        break;
    }

    return 0;
}

static int analize_symbol_usage(struct symbol_table *table, struct message_list *messages)
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
    DECL_FUNC
};
struct declaration {
    int kind;
    const char *ident;
    struct data_type *type;
};

static void make_decl(const struct ast_node *tree, struct declaration *decl);
static void make_decl2(const struct ast_node *tree, struct symbol_table *table);
static void add_symbol3(const struct ast_node *tree, struct symbol_table *table);

static void add_symbol3(const struct ast_node *tree, struct symbol_table *table)
{
    if (!tree)
        return;

    switch (tree->kind) {

    case NOD_DECL:
        make_decl2(tree, table);
        break;

    default:
        break;
    }

    add_symbol3(tree->l, table);
    add_symbol3(tree->r, table);
}

static void make_decl2(const struct ast_node *tree, struct symbol_table *table)
{
    struct declaration decl = {0};
    struct symbol *sym = NULL;
    int sym_kind = SYM_VAR;

    /*
    decl.kind = DECL_VAR;
    decl.ident = NULL;
    decl.type = NULL;
    */

    make_decl(tree, &decl);
#if 0
    printf("\n");
    printf("declaration:\n");
    printf("    kind:  %d\n", decl.kind);
    printf("    ident: %s\n", decl.ident);
    printf("    type:  %s\n", data_type_to_string(decl.type));
#endif

    switch (decl.kind) {
    case DECL_VAR:
        sym_kind = SYM_VAR;
        break;

    case DECL_FUNC:
        sym_kind = SYM_FUNC;
        break;

    default:
        break;
    }

    sym = define_symbol(table, decl.ident, sym_kind);
    sym->dtype = decl.type;
}

/* walk declaration tree in backwards */
static void make_decl(const struct ast_node *tree, struct declaration *decl)
{
    if (!tree)
        return;

    switch (tree->kind) {

    case NOD_DECL:
        printf("declaration: ");
        break;

    case NOD_DECL_FUNC:
        decl->kind = DECL_FUNC;
        printf(" function(");
        make_decl(tree->r, decl);
        printf(") returning");
        make_decl(tree->l, decl);
        return;

    case NOD_TYPE_ARRAY:
        if (tree->data.ival > 0)
            printf(" array %d of", tree->data.ival);
        else
            printf(" array of");
        decl->type = type_array(decl->type, tree->data.ival);
        break;

    case NOD_TYPE_POINTER:
        printf(" pointer to");
        printf("(pointer to %s)", data_type_to_string(decl->type));
        decl->type = type_ptr(decl->type);
        break;

    case NOD_TYPE_CHAR:
        printf(" char");
        decl->type = type_char();
        break;

    case NOD_TYPE_INT:
        printf(" int");
        decl->type = type_int();
        break;

    case NOD_TYPE_STRUCT:
        printf(" struct %s", tree->sval);
        break;

    case NOD_DECL_IDENT:
        printf("%s is", tree->sval);
        decl->ident = tree->sval;
        break;

    default:
        break;
    }

    make_decl(tree->r, decl);
    make_decl(tree->l, decl);
}

int semantic_analysis(struct ast_node *tree,
        struct symbol_table *table, struct message_list *messages)
{
    if (1) {

    add_symbol(tree, table);

    analize_symbol_usage(table, messages);

    symbol_assign_local_storage(table);

    add_type(tree, table);

    } else {

    add_symbol3(tree, table);

    }
    
    return 0;
}
