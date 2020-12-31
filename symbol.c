#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbol.h"

static void init_symbol(struct symbol *sym)
{
    struct symbol ini = {0};

    *sym = ini;
}

int is_global_var(const struct symbol *sym)
{
    if (sym->kind == SYM_VAR &&
        sym->scope_level == 0)
        return 1;
    else
        return 0;
}

int is_local_var(const struct symbol *sym)
{
    if (sym->kind == SYM_VAR &&
        sym->scope_level > 0)
        return 1;
    else
        return 0;
}

int is_param(const struct symbol *sym)
{
    return sym->kind == SYM_PARAM;
}

int is_func(const struct symbol *sym)
{
    return sym->kind == SYM_FUNC;
}

int is_enumerator(const struct symbol *sym)
{
    return sym->kind == SYM_ENUMERATOR;
}

struct symbol_table *new_symbol_table()
{
    struct symbol_table *table;
    int i;

    table = malloc(sizeof(struct symbol_table));

    for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        init_symbol(&table->data[i]);
    }

    table->symbol_count = 0;
    /* 0 means global scope */
    table->current_scope_level = 0;

    table->head = NULL;
    table->tail = NULL;

    return table;
}

void free_list(struct symbol *sym)
{
    if (!sym)
        return;
    free_list(sym->next);
    free(sym);
}

void free_symbol_table(struct symbol_table *table)
{
    if (!table)
        return;
    free_list(table->head);
    free(table);
}

#define SYM_LIST(S) \
    S(SYM_SCOPE_BEGIN) \
    S(SYM_SCOPE_END) \
    S(SYM_VAR) \
    S(SYM_FUNC) \
    S(SYM_PARAM) \
    S(SYM_MEMBER) \
    S(SYM_ENUMERATOR) \
    S(SYM_TAG_STRUCT) \
    S(SYM_TAG_ENUM)

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

static void print_horizonal_line(char c, int n)
{
    int i;
    for (i = 0; i < n; i++)
        printf("%c", c);
    printf("\n");
}

void print_symbol_table(const struct symbol_table *table)
{
    const int ROW = 79;
    const struct symbol *sym;

    print_horizonal_line('-', ROW);

    printf("|");
    printf("%15s | ", "name");
    printf("%20s | ", "kind");
    printf("%10s | ", "type");
    printf("%5s | ", "level");
    printf("%5s | ", "mem");
    printf("%6s | ", "DDRIAU");
    printf("\n");

    print_horizonal_line('=', ROW);

    for (sym = table->head; sym; sym = sym->next) {
        printf("|");
        printf("%15s | ", sym->name ? sym->name : "--");
        printf("%-20s | ",  symbol_to_string(sym));
        printf("%-10s | ", data_type_to_string(sym->type));
        printf("%5d | ", sym->scope_level);
        if (is_global_var(sym))
            printf("%5s | ",  "*");
        else
            printf("%5d | ",  sym->mem_offset);

        printf("%c", sym->is_declared    ? '*' : '.');
        printf("%c", sym->is_defined     ? '*' : '.');
        printf("%c", sym->is_redefined   ? '*' : '.');
        printf("%c", sym->is_initialized ? '*' : '.');
        printf("%c", sym->is_assigned    ? '*' : '.');
        printf("%c", sym->is_used        ? '*' : '.');
        printf(" |");

        printf("\n");
    }

    print_horizonal_line('-', ROW);
}

static struct symbol *new_symbol()
{
    struct symbol *sym = malloc(sizeof(struct symbol));
    init_symbol(sym);
    return sym;
}

static struct symbol *push_symbol(struct symbol_table *table, const char *name, int kind)
{
    struct symbol *sym = new_symbol();
    sym->kind = kind;
    sym->name = name;
    sym->scope_level = table->current_scope_level;

    if (!table->head) {
        table->head = sym;
        table->tail = sym;
        return sym;
    }

    table->tail->next = sym;
    sym->prev = table->tail;
    table->tail = sym;

    table->symbol_count++;
    return sym;
}

void init_symbol_table(struct symbol_table *table)
{
    int i;

    for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        init_symbol(&table->data[i]);
    }

    table->symbol_count = 0;
    /* 0 means global scope */
    table->current_scope_level = 0;

    table->head = NULL;
    table->tail = NULL;
}

static int namespace(int kind)
{
    switch (kind) {

    case SYM_SCOPE_BEGIN:
    case SYM_SCOPE_END:
        return 0;

    case SYM_VAR:
    case SYM_FUNC:
    case SYM_PARAM:
    case SYM_ENUMERATOR:
        return 1;

    case SYM_TAG_STRUCT:
    case SYM_TAG_ENUM:
        return 2;

    case SYM_MEMBER:
        return 3;

    default:
        return 0;
    }
}

static int is_same_namespace(int kind0, int kind1)
{
    return namespace(kind0) == namespace(kind1);
}

struct symbol *lookup_symbol(struct symbol_table *table,
        const char *name, enum symbol_kind kind)
{
    struct symbol *sym;
    /* lowest level so far during search */
    int lv_low = table->current_scope_level;

    /* search backwards */
    for (sym = table->tail; sym; sym = sym->prev) {

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

        if (!strcmp(sym->name, name) && is_same_namespace(sym->kind, kind)) {
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

    return sym;
}

struct symbol *use_symbol(struct symbol_table *table, const char *name, int kind)
{
    struct symbol *sym = lookup_symbol(table, name, kind);

    if (!sym) {
        sym = push_symbol(table, name, kind);
        sym->type = type_int();
    }

    /* symbol usage will be checked later */
    /* sym->is_used = 1; */
    return sym;
}

struct symbol *define_symbol(struct symbol_table *table, const char *name, int kind)
{
    struct symbol *sym = lookup_symbol(table, name, kind);
    const int cur_lv = table->current_scope_level;

    if (sym && sym->scope_level == cur_lv) {
        sym->is_redefined = 1;
        return sym;
    }

    sym = push_symbol(table, name, kind);
    sym->is_defined = 1;

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

struct symbol *begin(struct symbol_table *table)
{
    return table->head;
}

struct symbol *end(struct symbol_table *table)
{
    return NULL;
}

struct symbol *next(struct symbol *sym)
{
    return sym->next;
}

struct symbol *prev(struct symbol *sym)
{
    return sym->prev;
}
