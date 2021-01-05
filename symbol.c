#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbol.h"

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

int is_label(const struct symbol *sym)
{
    return sym->kind == SYM_LABEL;
}

struct symbol_table *new_symbol_table()
{
    struct symbol_table *table;
    table = malloc(sizeof(struct symbol_table));

    table->head = NULL;
    table->tail = NULL;
    /* 0 means global scope */
    table->current_scope_level = 0;
    table->current_switch_level = 1000;

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

const char *symbol_to_string(const struct symbol *sym)
{
    if (!sym)
        return "null";

    switch (sym->kind) {
    case SYM_SCOPE_BEGIN: return "SYM_SCOPE_BEGIN";
    case SYM_SCOPE_END: return "SYM_SCOPE_END";
    case SYM_SWITCH_BEGIN: return "SYM_SWITCH_BEGIN";
    case SYM_SWITCH_END: return "SYM_SWITCH_END";
    case SYM_CASE: return "SYM_CASE";
    case SYM_DEFAULT: return "SYM_DEFAULT";
    case SYM_LABEL: return "SYM_LABEL";
    case SYM_VAR: return "SYM_VAR";
    case SYM_FUNC: return "SYM_FUNC";
    case SYM_PARAM: return "SYM_PARAM";
    case SYM_MEMBER: return "SYM_MEMBER";
    case SYM_ENUMERATOR: return "SYM_ENUMERATOR";
    case SYM_TAG_STRUCT: return "SYM_TAG_STRUCT";
    case SYM_TAG_ENUM: return "SYM_TAG_ENUM";
    default: return "**unknown**";
    }
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
    const int ROW = 87;
    const struct symbol *sym;

    print_horizonal_line('-', ROW);

    printf("|");
    printf("%15s | ", "name");
    printf("%20s | ", "kind");
    printf("%10s | ", "type");
    printf("%5s | ", "level");
    printf("%5s | ", "mem");
    printf("%5s | ", "id");
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
        printf("%5d | ",  sym->id);

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
    static int id = 0;
    struct symbol ini = {0};
    struct symbol *sym = malloc(sizeof(struct symbol));
    *sym = ini;
    sym->id = id++;
    return sym;
}

static struct symbol *push_symbol(struct symbol_table *table,
        const char *name, int kind, struct data_type *type)
{
    struct symbol *sym = new_symbol();
    sym->kind = kind;
    sym->name = name;
    sym->type = type;
    sym->scope_level = table->current_scope_level;

    if (!table->head) {
        table->head = sym;
        table->tail = sym;
        return sym;
    }

    table->tail->next = sym;
    sym->prev = table->tail;
    table->tail = sym;

    return sym;
}

static int namespace_of(int kind)
{
    switch (kind) {

    case SYM_SCOPE_BEGIN:
    case SYM_SCOPE_END:
    case SYM_SWITCH_BEGIN:
    case SYM_SWITCH_END:
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

static int match_namespace(const struct symbol *sym, int kind)
{
    if (!sym)
        return 0;
    return namespace_of(sym->kind) == namespace_of(kind);
}

static int match_name(const struct symbol *sym, const char *name)
{
    if (!sym->name || !name)
        return 0;
    return !strcmp(sym->name, name);
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
        if (sym->kind == SYM_SCOPE_END)
            continue;

        /* skip upper level scope */
        if (sym->scope_level > lv_low)
            continue;

        if (match_name(sym, name) && match_namespace(sym, kind))
            return sym;
    }

    return NULL;
}

struct symbol *insert_symbol(struct symbol_table *table,
        const char *name, enum symbol_kind kind)
{
    struct symbol *sym = NULL;

    if (lookup_symbol(table, name, kind) != NULL)
        return NULL;

    return sym;
}

struct symbol *use_symbol(struct symbol_table *table, const char *name, int kind)
{
    struct symbol *sym = lookup_symbol(table, name, kind);

    if (!sym)
        sym = push_symbol(table, name, kind, type_int());

    return sym;
}

struct symbol *define_symbol(struct symbol_table *table,
        const char *name, int kind, struct data_type *type)
{
    struct symbol *sym = lookup_symbol(table, name, kind);
    const int cur_lv = table->current_scope_level;

    if (sym && sym->scope_level == cur_lv) {
        sym->is_redefined = 1;
        return sym;
    }

    sym = push_symbol(table, name, kind, type);
    sym->is_defined = 1;

    return sym;
}

struct symbol *define_case_symbol(struct symbol_table *table, int kind)
{
    struct symbol *sym;

    if (kind != SYM_CASE && kind != SYM_DEFAULT)
        return NULL;

    sym = push_symbol(table, NULL, kind, NULL);

    sym->scope_level = table->current_switch_level;
    sym->is_defined = 1;

    return sym;
}

struct symbol *define_label_symbol(struct symbol_table *table, const char *label)
{
    struct symbol *sym;
    struct symbol *func_sym = NULL;

    for (sym = table->tail; sym; sym = sym->prev) {

        if (match_name(sym, label)) {
            sym->is_redefined = 1;
            return sym;
        }

        /* reached function sym */
        if (is_func(sym)) {
            func_sym = sym;
            break;
        }
    }

    sym = new_symbol();
    sym->kind = SYM_LABEL;
    sym->name = label;
    sym->type = type_int();
    sym->is_defined = 1;

    sym->next = func_sym->next;
    func_sym->next->prev = sym;
    func_sym->next = sym;
    sym->prev = func_sym;

    return sym;
}

struct symbol *use_label_symbol(struct symbol *func_sym, const char *label)
{
    struct symbol *sym;

    for (sym = func_sym; sym; sym = sym->next) {
        if (match_name(sym, label))
            break;

        /* end of function */
        if (sym->kind == SYM_SCOPE_END && sym->scope_level == 1) {
            sym = NULL;
            break;
        }
    }

    if (!sym) {
        sym = new_symbol();
        sym->kind = SYM_LABEL;
        sym->name = label;
        sym->type = type_int();

        sym->next = func_sym->next;
        func_sym->next->prev = sym;
        func_sym->next = sym;
        sym->prev = func_sym;
    }
    return sym;
}

int symbol_scope_begin(struct symbol_table *table)
{
    table->current_scope_level++;
    push_symbol(table, NULL, SYM_SCOPE_BEGIN, NULL);

    return table->current_scope_level;
}

int symbol_scope_end(struct symbol_table *table)
{
    push_symbol(table, NULL, SYM_SCOPE_END, NULL);
    table->current_scope_level--;

    return table->current_scope_level;
}

int symbol_switch_begin(struct symbol_table *table)
{
    struct symbol *sym;

    table->current_switch_level++;
    sym = push_symbol(table, NULL, SYM_SWITCH_BEGIN, NULL);
    sym->scope_level = table->current_switch_level;

    return table->current_switch_level;
}

int symbol_switch_end(struct symbol_table *table)
{
    struct symbol *sym;

    sym = push_symbol(table, NULL, SYM_SWITCH_END, NULL);
    sym->scope_level = table->current_switch_level;
    table->current_switch_level--;

    return table->current_switch_level;
}
