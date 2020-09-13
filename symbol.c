#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbol.h"

static void init_symbol(struct symbol *sym)
{
    sym->name = NULL;
    sym->kind = SYM_SCOPE_BEGIN;
    sym->mem_offset = 0;

    /*
    sym->local_var_id = 0;
    */
    sym->scope_level = 0;
}

/* XXX */
struct symbol_table *new_symbol_table()
{
    struct symbol_table *table;
    int i;

    table = malloc(sizeof(struct symbol_table));

    for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        init_symbol(&table->data[i]);
    }

    table->symbol_count = 0;
    /*
    table->local_var_id = 0;
    */
    /* 0 means global scope */
    table->current_scope_level = 0;

    return table;
}

void free_symbol_table(struct symbol_table *table)
{
    if (table) {
        free(table);
    }
}

static struct symbol *push_symbol(struct symbol_table *table, const char *name, int kind)
{
    struct symbol *sym = &table->data[table->symbol_count++];
    sym->kind = kind;
    sym->scope_level = table->current_scope_level;

    if (name != NULL) {
        sym->name = malloc(strlen(name) + 1);
        strcpy(sym->name, name);
    }

    return sym;
}

void init_symbol_table(struct symbol_table *table)
{
    int i;

    for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        init_symbol(&table->data[i]);
    }

    table->symbol_count = 0;

    /*
    table->local_var_id = 0;
    */
    /* 0 means global scope */
    table->current_scope_level = 0;
}

struct symbol *lookup_symbol(struct symbol_table *table,
        const char *name, enum symbol_kind kind)
{
    /* lowest level so far during search */
    int lv_low = table->current_scope_level;
    int i;

    /* search backwards */
    for (i = table->symbol_count - 1; i >= 0; i--) {
        struct symbol *sym = &table->data[i];

        /* step down one level */
        if (sym->kind == SYM_SCOPE_BEGIN) {
            const int lv_outside = sym->scope_level - 1;
            lv_low = lv_outside < lv_low ? lv_outside : lv_low;
            continue;
        }

        /* step up one level */
        if (sym->kind == SYM_SCOPE_END) {
            continue;
        }

        /* skip upper level scope */
        if (sym->scope_level > lv_low) {
            continue;
        }

        if (!strcmp(sym->name, name) /*&& sym->kind == kind*/) {
            return sym;
        }
    }

    return NULL;
}

struct symbol *insert_symbol(struct symbol_table *table,
        const char *name, enum symbol_kind kind)
{
    struct symbol *sym = NULL;

    if (lookup_symbol(table, name, kind) != NULL) {
        return NULL;
    }

    if (kind == SYM_FUNC) {
        /* reset id */
        /*
        table->local_var_id = 0;
        */
    }

    sym = push_symbol(table, name, kind);

    if (kind == SYM_VAR || kind == SYM_PARAM) {
        /*
        sym->local_var_id = table->local_var_id++;
        */
    }

    return sym;
}

int symbol_scope_begin(struct symbol_table *table)
{
    table->current_scope_level++;
    push_symbol(table, NULL, SYM_SCOPE_BEGIN);

    return table->current_scope_level;
}

int symbol_scope_end(struct symbol_table *table)
{
    push_symbol(table, NULL, SYM_SCOPE_END);
    table->current_scope_level--;

    return table->current_scope_level;
}

static int next_aligned(int current_offset, int next_align)
{
    int next_offset = current_offset;

    if (current_offset % next_align > 0) {
        const int padding = next_align - current_offset % next_align;
        next_offset += padding;
    }

    return next_offset;
}

int symbol_assign_local_storage(struct symbol_table *table)
{
    int i;
    int N = table->symbol_count;
    int total_mem_offset = 0;
    struct symbol *func = NULL;

    for (i = 0; i < N; i++) {
        struct symbol *sym = &table->data[i];

        if (sym->kind == SYM_FUNC) {
            func = sym;
            total_mem_offset = 0;
            continue;
        }

        if (sym->kind == SYM_SCOPE_BEGIN) {
            continue;
        }

        if (sym->kind == SYM_SCOPE_END) {
            if (sym->scope_level == 1) {
                /* end of function */
                func->mem_offset = next_aligned(total_mem_offset, 16);
            }
            continue;
        }

        if (sym->kind == SYM_VAR || sym->kind == SYM_PARAM) {
            const int size  = sym->dtype->byte_size;
            const int align = sym->dtype->alignment;
            const int len   = sym->dtype->array_len;

            total_mem_offset = next_aligned(total_mem_offset, align);
            total_mem_offset += len * size;

            sym->mem_offset = total_mem_offset;
        }
    }
    return 0;
}
