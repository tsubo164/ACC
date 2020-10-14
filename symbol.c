#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbol.h"

static void init_symbol(struct symbol *sym)
{
    sym->name = NULL;
    sym->kind = SYM_SCOPE_BEGIN;
    sym->flag = 0;
    sym->mem_offset = 0;

    /*
    sym->local_var_id = 0;
    */
    sym->scope_level = 0;

    /* XXX */
    sym->file_pos = 0L;
}

int is_global_var(const struct symbol *sym)
{
    if (sym->kind == SYM_VAR &&
        sym->scope_level == 0)
        return 1;
    else
        return 0;
}

int is_param(const struct symbol *sym)
{
    return sym->kind == SYM_PARAM;
}

void symbol_flag_on(struct symbol *sym, int flag)
{
    sym->flag |= flag;
}

int symbol_flag_is_on(const struct symbol *sym, int flag)
{
    return sym->flag & flag;
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

#define SYM_LIST(S) \
    S(SYM_SCOPE_BEGIN) \
    S(SYM_SCOPE_END) \
    S(SYM_STRUCT) \
    S(SYM_VAR) \
    S(SYM_GLOBAL_VAR) \
    S(SYM_FUNC) \
    S(SYM_PARAM)

const char *symbol_to_string(const struct symbol *sym)
{
    if (!sym)
        return "null";

#define S(kind) case kind: return #kind;
    switch (sym->kind) {
SYM_LIST(S)
    default: return "**unknown**";
    }
#undef S
}

void print_symbol_table(const struct symbol_table *table)
{
    const int N = table->symbol_count;
    int i;

    printf("%15s | ", "name");
    printf("%20s | ", "kind");
    printf("%5s | ", "level");
    printf("\n");
    printf("================================================\n");

    for (i = 0; i < N; i++) {
        const struct symbol *sym = &table->data[i];
        printf("%15s | ", sym->name);
        printf("%-20s | ",  symbol_to_string(sym));
        printf("%5d | ", sym->scope_level);
        printf("\n");
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

struct symbol *use_symbol(struct symbol_table *table, const char *name, int kind)
{
    struct symbol *sym = lookup_symbol(table, name, kind);

    if (!sym) {
        sym = push_symbol(table, name, kind);
        symbol_flag_on(sym, IS_USED);
    }

    return sym;
}

struct symbol *define_symbol(struct symbol_table *table, const char *name, int kind)
{
    struct symbol *sym = lookup_symbol(table, name, kind);
    const int cur_lv = table->current_scope_level;

    if (sym && sym->scope_level == cur_lv) {
        symbol_flag_on(sym, IS_REDEFINED);
        return sym;
    }

    sym = push_symbol(table, name, kind);

    return sym;
}

struct symbol *define_variable(struct symbol_table *table, const char *name)
{
    struct symbol *sym = lookup_symbol(table, name, SYM_VAR);
    const int cur_lv = table->current_scope_level;

    if (sym && sym->scope_level == cur_lv) {
        symbol_flag_on(sym, IS_REDEFINED);
        return sym;
    }

    sym = push_symbol(table, name, SYM_VAR);

    return sym;
}

struct symbol *define_function(struct symbol_table *table, const char *name)
{
    struct symbol *sym = lookup_symbol(table, name, SYM_FUNC);
    const int cur_lv = table->current_scope_level;

    if (sym && sym->scope_level == cur_lv) {
        symbol_flag_on(sym, IS_REDEFINED);
        return sym;
    }

    sym = push_symbol(table, name, SYM_FUNC);

    return sym;
}

struct symbol *define_struct(struct symbol_table *table, const char *name)
{
    struct symbol *sym = lookup_symbol(table, name, SYM_STRUCT);
    const int cur_lv = table->current_scope_level;

    if (sym && sym->scope_level == cur_lv) {
        symbol_flag_on(sym, IS_REDEFINED);
        return sym;
    }

    sym = push_symbol(table, name, SYM_STRUCT);

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

int get_symbol_count(const struct symbol_table *table)
{
    return table->symbol_count;
}

struct symbol *get_symbol(struct symbol_table *table, int index)
{
    if (index < 0 || index >= get_symbol_count(table)) {
        return NULL;
    }
    return &table->data[index];
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
    struct symbol *struct_ = NULL;

    for (i = 0; i < N; i++) {
        struct symbol *sym = &table->data[i];

        if (sym->kind == SYM_FUNC) {
            func = sym;
            total_mem_offset = 0;
            continue;
        }

        if (sym->kind == SYM_STRUCT) {
            struct_ = sym;
            total_mem_offset = 0;
            continue;
        }

        if (sym->kind == SYM_SCOPE_BEGIN) {
            continue;
        }

        if (sym->kind == SYM_SCOPE_END) {
            /* XXX */
            if (sym->scope_level == 1 && func) {
                /* end of function. backpatching memoffset for local vars */
                func->mem_offset = next_aligned(total_mem_offset, 16);
                func = NULL;
            }
            /* XXX */
            if (/* sym->scope_level == 1 && */ struct_) {
                /* end of function. backpatching memoffset for local vars */
                struct_->mem_offset = next_aligned(total_mem_offset, 8);
                {
                    struct data_type *dtype = type_struct(struct_->name);
                    dtype->byte_size = struct_->mem_offset;
                    dtype->alignment = 8;
                    dtype->array_len = 1;
                    struct_->dtype = dtype;
                    /*
                    */
                }
                struct_ = NULL;
            }
            continue;
        }

        if (sym->kind == SYM_VAR || sym->kind == SYM_PARAM) {
            int size  = sym->dtype->byte_size;
            int align = sym->dtype->alignment;
            int len   = sym->dtype->array_len;

            if (sym->dtype->kind == DATA_TYPE_STRUCT) {
                struct symbol *strc = lookup_symbol(table, sym->dtype->tag, SYM_STRUCT);
                size  = strc->dtype->byte_size;
                align = strc->dtype->alignment;
                len   = strc->dtype->array_len;
            }
                /*
    printf("    %s\n", sym->name);
    printf("        %d\n", sym->kind);
    printf("        %d\n", size);
    printf("        %d\n", align);
    printf("        %d\n", len);
                */

                /*
            printf("        total_mem_offset: %d\n", total_mem_offset);
            printf("        align           : %d\n", align);
                */
            total_mem_offset = next_aligned(total_mem_offset, align);
                /*
            printf("        total_mem_offset: %d\n", total_mem_offset);
                */
            total_mem_offset += len * size;

            sym->mem_offset = total_mem_offset;
                /*
            printf("        total_mem_offset: %d\n", total_mem_offset);
                */
        }
    }
    return 0;
}
