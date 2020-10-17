#include <stdio.h>
#include <string.h>
#include "semantics.h"
#include "message.h"

#if 0
static void promote_data_type2(struct ast_node *node)
{
    const struct ast_node *n1, *n2;

    if (!node) {
        return;
    }

    n1 = node->l;
    n2 = node->r;

    if (!n1 && !n2) {
        return;
    }

    if (!n1) {
        node->dtype = n2->dtype;
        return;
    }

    if (!n2) {
        node->dtype = n1->dtype;
        return;
    }

    /*
    printf("    [%s]\n", node_to_string(n1));
    printf("            =><%p>\n", (void *)n1->l);
    printf("            =><%p>\n", (void *)n1->r);
    printf("            =><%p>\n", (void *)n1->dtype);
    printf("            =><%p>\n", (void *)n2->dtype);
    */

    if (n1->dtype->kind > n2->dtype->kind) {
        node->dtype = n1->dtype;
        return;
    } else {
        node->dtype = n2->dtype;
        return;
    }
}
#endif

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

    /*
    printf("    [%s]\n", node_to_string(n1));
    printf("            =><%p>\n", (void *)n1->l);
    printf("            =><%p>\n", (void *)n1->r);
    printf("            =><%p>\n", (void *)n1->dtype);
    printf("            =><%p>\n", (void *)n2->dtype);
    */

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

#define TEST 0
static void scope_begin(struct symbol_table *table)
{
#if TEST
    printf("     {\n");
#else
    symbol_scope_begin(table);
#endif
}

static void scope_end(struct symbol_table *table)
{
#if TEST
    printf("     }\n");
#else
    symbol_scope_end(table);
#endif
}

static void define_sym(struct ast_node *node, struct symbol_table *table, int sym_kind)
{
#if TEST
    printf("     [%s]\n", node->sval);
#else
    const char *name = node->sval;
    struct symbol *sym = define_symbol(table, name, sym_kind);

    sym->dtype = make_type(node->l);
/*
    printf("   NODE: %s: %s: ", node_to_string(node), sym->name);
    printf("%s\n", data_type_to_string(sym->dtype));
    if (sym->dtype) {
        printf("    %s\n", sym->dtype->tag);
    }
*/
    /*
    ast_node_set_symbol(node, sym);
    */
    node->data.sym = sym;
#endif
}

static void use_sym(struct ast_node *node, struct symbol_table *table, int sym_kind)
{
#if TEST
    printf("     %s\n", node->sval);
#else
    const char *name = node->sval;
    struct symbol *sym = use_symbol(table, name, sym_kind);

    /*
    ast_node_set_symbol(node, sym);
    */
    node->data.sym = sym;

    /* TODO find better way */
    if (is_param(sym)) {
        node->kind = NOD_PARAM;
    } else if (is_global_var(sym)) {
        node->kind = NOD_GLOBAL_VAR;
    }
/*
    printf("\n");
    printf("   NODE: %s: %s: ", node_to_string(node), sym->name);
    printf("%s\n", data_type_to_string(sym->dtype));
    if (sym->dtype) {
        printf("    %s\n", sym->dtype->tag);
    }
    printf("    %p\n", (void *) node->dtype);
*/
#endif
}

static void add_symbols(struct ast_node *node, struct symbol_table *table)
{
    if (!node)
        return;

    switch (node->kind) {

        /*
    case NOD_GLOBAL_VAR:
    case NOD_PARAM:
        */
    case NOD_VAR:
        use_sym(node, table, SYM_VAR);
        add_symbols(node->l, table);
        add_symbols(node->r, table);
        break;

    case NOD_PARAM_DEF:
        define_sym(node, table, SYM_PARAM);
        add_symbols(node->l, table);
        add_symbols(node->r, table);
        break;

    case NOD_CALL:
        use_sym(node, table, SYM_FUNC);
        add_symbols(node->l, table);
        add_symbols(node->r, table);
        break;

    case NOD_VAR_DEF:
        define_sym(node, table, SYM_VAR);
        add_symbols(node->l, table);
        add_symbols(node->r, table);
        break;

    case NOD_FUNC_DEF:
        define_sym(node, table, SYM_FUNC);
        scope_begin(table);
        add_symbols(node->l, table);
        add_symbols(node->r, table);
        scope_end(table);
        break;

    case NOD_STRUCT_REF:
        {
            const struct symbol *sym;
            const char *mem = node->r->sval;
            const char *tag;

            add_symbols(node->l, table);
        /*
            printf("HOGE\n");
            printf("   sym: %s\n", node->l->data.sym->name);
            printf("   sym: %s\n", data_type_to_string(node->l->dtype));
            printf("     %p\n", (void *) node->l->dtype);
            printf("   sym: %s\n", node->l->dtype->tag);
            printf("   node: %s\n", node_to_string(node->l));
        */
        /*
            printf("   sym: %d\n", node->l->dtype->kind);
        */
            /* TODO we can not use node->dtype because dtype is not set yet
             * when adding symbol
             * should define sym_of(node) or symbol_(node)
             * text_(node), int_(node) ...
             * set_text(node, ...), set_int(node, ...)
             * L_(node), R_(node)
             */
            tag = node->l->data.sym->dtype->tag;
            /*
            tag = node->l->dtype->tag;
            */

            sym = lookup_symbol(table, tag, SYM_STRUCT);

            for (;;) {
                if (sym->kind == SYM_SCOPE_END) {
                    /* end of struct definition */
                    break;
                }

                if (sym->name && !strcmp(sym->name, mem)) {
                    node->r->data.sym = sym;
                    /* will do later */
                    /*
                    node->dtype = node->r->dtype;
                    */

                    /* struct var data type <- member data type */
                    /* will do later */
                    /*
                    node->dtype = sym->dtype;
                    */
                    node->r->dtype = sym->dtype;

                    break;
                }
                sym++;
            }
#if 0
#endif
        }
        break;

    case NOD_STRUCT_DECL:
        define_sym(node, table, SYM_STRUCT);
        scope_begin(table);
        add_symbols(node->l, table);
        add_symbols(node->r, table);
        scope_end(table);
        break;

    case NOD_COMPOUND:
        scope_begin(table);
        add_symbols(node->l, table);
        add_symbols(node->r, table);
        scope_end(table);
        break;

    default:
        add_symbols(node->l, table);
        add_symbols(node->r, table);
        break;
    }
}

static int promote_type2(struct ast_node *node, struct symbol_table *table)
{
    if (!node) {
        return 0;
    }

    promote_type2(node->l, table);
    promote_type2(node->r, table);

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
        } else {
            /*
            node->dtype = promote_data_type(node->l, node->r);
            */
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
        /*
        */
        break;

    case NOD_NUM:
        node->dtype = type_int();
        /*
        node->data.ival = tok->value;
        printf(">>> %s, %d\n", node->sval, node->data.ival);
        */
        break;

    default:
        break;
    }

    return 0;
}

int promote_type(struct ast_node *node, struct symbol_table *table)
{
    if (!node) {
        return 0;
    }

    /* void */
    /*
    if (node->dtype == NULL) {
        node->dtype = type_void();
    }
    */

    /*
    printf("    [%s]\n", node_to_string(node));
    printf("            => [%s]\n", data_type_to_string(node->dtype));
    printf("            => [%p]\n", (void *)node->dtype);
    printf("            => [%p]\n", (void *)node->l);
    printf("            => [%p]\n", (void *)node->r);
    */
    /*
    */


    promote_type(node->l, table);
    promote_type(node->r, table);

    /* promote */
    /*
    promote_data_type2(node);
    printf("    [%s]\n", node_to_string(node));
    printf("            => [%s]\n", data_type_to_string(node->dtype));
    node->dtype = promote_data_type(node->l, node->r);
    printf("    <%s>\n", node_to_string(node));
    printf("            => <%s>\n", data_type_to_string(node->dtype));
    */

    /*
    if (node->data.sym) {
        node->dtype = node->data.sym->dtype;
        return 0;
    }

    if (node->dtype == NULL) {
        node->dtype = type_void();
        return 0;
    }
    node->dtype = promote_data_type(node->l, node->r);
    */

    /*
    */
    switch (node->kind) {

    case NOD_ADD:
        node->dtype = promote_data_type(node->l, node->r);
        /*
        */

        if (node->l->dtype->kind == DATA_TYPE_ARRAY) {
            struct ast_node *size, *mul;

            size = new_ast_node(NOD_NUM, NULL, NULL);
            size->data.ival = node->l->dtype->ptr_to->byte_size;
            mul = new_ast_node(NOD_MUL, size, node->r);
            node->r = mul;
        } else {
            /*
            node->dtype = promote_data_type(node->l, node->r);
            */
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
        /*
        printf("    [%s]\n", data_type_to_string(node->dtype));
        printf("        [%s]\n", data_type_to_string(node->l->dtype));
        printf("            <%d>\n", node->l->dtype->kind);
        printf("        [%s]\n", data_type_to_string(node->r->dtype));
        printf("            <%d>\n", node->r->dtype->kind);
        */
        break;

    case NOD_ASSIGN:
        node->dtype = node->l->dtype;
        break;
        /*
    case NOD_NUM:
        break;
        */

        /*
    case NOD_VAR:
        node->dtype = node->data.sym->dtype;
        break;
        */

    case NOD_DEREF:
        node->dtype = promote_data_type(node->l, node->r);
        /*
        */
        node->dtype = node->dtype->ptr_to;;
        break;
#if 0
#endif
    case NOD_STRUCT_REF:
        /*
        printf("    node->dtype: %s\n", node->l->data.sym->name);
        printf("    node->dtype: %d\n", node->l->dtype->kind);
        */
        {
            const struct symbol *sym;
            const char *mem = node->r->sval;
            /*
            printf("    struct tag: %s\n", node->l->data.sym->dtype->tag);
            printf("        member: %s\n", mem);
            */

            sym = lookup_symbol(table, node->l->data.sym->dtype->tag, SYM_STRUCT);
        /*
            printf("    struct tag: %s\n", sym->dtype->tag);
            printf("%d\n", node->l->dtype->kind);
        */
            for (;;) {
                if (sym->kind == SYM_SCOPE_END) {
                    /* end of struct definition */
                    break;
                }

                if (sym->name && !strcmp(sym->name, mem)) {
                    node->r->data.sym = sym;
                    node->dtype = node->r->dtype;
                    /*
                    printf("%d\n", sym->dtype->kind);
                    */

                    /* struct var data type <- member data type */
                    node->dtype = sym->dtype;
                    node->r->dtype = sym->dtype;

                    break;
                }
                sym++;
            }
        }
        /*
        printf("    node->dtype: %d\n", node->l->dtype->kind);
        printf("    node->dtype: %d\n", node->r->dtype->kind);
        printf("    node->dtype: %d\n", node->dtype->kind);
        */
        break;

    default:
        /*
        if (node->dtype == NULL) {
            node->dtype = type_void();
        }
        */
        /*
        else if (node->data.sym) {
            printf("--------%d\n", node->data.sym->kind);
            node->dtype = node->data.sym->dtype;
        }
        */
        break;
    }

#if 0
    printf("    [%s]\n", data_type_to_string(node->dtype));
    printf("        <%d>\n", node->dtype->kind);
    /*
    */
#endif

    /*
    printf("    [%s]\n", node_to_string(node));
    printf("            => [%s]\n", data_type_to_string(node->dtype));
    */

    return 0;
}

#if 0
static int promote_type__(struct ast_node *node)
{
    if (!node) {
        return 0;
    }

    promote_type__(node->l);
    promote_type__(node->r);

    switch (node->kind) {

    case NOD_ADD:
    case NOD_SUB:
    case NOD_MUL:
    case NOD_DIV:
    case NOD_ASSIGN:

    case NOD_DEREF:
        node->dtype = promote_data_type(node->l, node->r);
        break;

    default:
        break;
    }

    /*
    promote_data_type2(node);
    printf("    [%s]\n", node_to_string(node));
    printf("            => [%s]\n", data_type_to_string(node->dtype));
    */

    return 0;
}
#endif

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

int semantic_analysis(struct ast_node *tree,
        struct symbol_table *table, struct message_list *messages)
{
    /*
    */

    {
        /*
        struct symbol_table *symtab = new_symbol_table();
        if (0) {
            add_symbols(tree, symtab);
            print_symbol_table(symtab);
        }
        free_symbol_table(symtab);
        */
    }
    /*
    print_symbol_table(table);
    */

    table->current_scope_level = 0;
    table->symbol_count = 0;
    add_symbols(tree, table);

    analize_symbol_usage(table, messages);
    /*
    print_symbol_table(table);
    */
    /*
    */
    symbol_assign_local_storage(table);
    /*
    print_symbol_table(table);
    */
    /* XXX */
    promote_type2(tree, table);
    /*
    promote_type(tree, table);
    */

    return 0;
}
