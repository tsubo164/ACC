#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbol.h"

int is_extern(const struct symbol *sym)
{
    return sym && sym->is_extern;
}

int is_static(const struct symbol *sym)
{
    return sym && sym->is_static;
}

int is_global_var(const struct symbol *sym)
{
    if (!sym)
        return 0;

    if (sym->kind == SYM_VAR &&
        sym->scope_level == 0)
        return 1;

    if (sym->kind == SYM_VAR &&
        sym->scope_level > 0 &&
        sym->is_static)
        return 1;
    else
        return 0;
}

int is_local_var(const struct symbol *sym)
{
    if (!sym)
        return 0;

    if (sym->kind == SYM_VAR &&
        sym->scope_level > 0 &&
        !sym->is_static)
        return 1;
    else
        return 0;
}

int is_func(const struct symbol *sym)
{
    return sym && sym->kind == SYM_FUNC;
}

int is_param(const struct symbol *sym)
{
    return sym && sym->kind == SYM_PARAM;
}

int is_struct_tag(const struct symbol *sym)
{
    return sym && sym->kind == SYM_TAG_STRUCT;
}

int is_member(const struct symbol *sym)
{
    return sym && sym->kind == SYM_MEMBER;
}

int is_enum_tag(const struct symbol *sym)
{
    return sym && sym->kind == SYM_TAG_ENUM;
}

int is_enumerator(const struct symbol *sym)
{
    return sym && sym->kind == SYM_ENUMERATOR;
}

int is_case(const struct symbol *sym)
{
    return sym && sym->kind == SYM_CASE;
}

int is_default(const struct symbol *sym)
{
    return sym && sym->kind == SYM_DEFAULT;
}

int is_label(const struct symbol *sym)
{
    return sym && sym->kind == SYM_LABEL;
}

int is_typedef(const struct symbol *sym)
{
    return sym && sym->kind == SYM_TYPEDEF;
}

int is_string_literal(const struct symbol *sym)
{
    return sym && sym->kind == SYM_STRING;
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
    case SYM_STRING: return "SYM_STRING";
    case SYM_VAR: return "SYM_VAR";
    case SYM_FUNC: return "SYM_FUNC";
    case SYM_PARAM: return "SYM_PARAM";
    case SYM_MEMBER: return "SYM_MEMBER";
    case SYM_ENUMERATOR: return "SYM_ENUMERATOR";
    case SYM_TAG_STRUCT: return "SYM_TAG_STRUCT";
    case SYM_TAG_ENUM: return "SYM_TAG_ENUM";
    case SYM_TYPEDEF: return "SYM_TYPEDEF";
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
    const int COLUMNS = 88;
    const struct symbol *sym;

    print_horizonal_line('-', COLUMNS);

    printf("|");
    printf("%15s | ", "name");
    printf("%20s | ", "kind");
    printf("%10s | ", "type");
    printf("%5s | ", "scope");
    printf("%5s| ", "offset");
    printf("%5s | ", "id");
    printf("%7s | ", "ESDRIAU");
    printf("\n");

    print_horizonal_line('=', COLUMNS);

    for (sym = table->head; sym; sym = sym->next) {
        printf("|");
        /* symbol name */
        printf("%15.15s | ", sym->name ? sym->name : "--");

        /* symbol kind */
        printf("%-20s | ",  symbol_to_string(sym));

        /* type */
        if (is_type_name(sym->type)) {
            printf("%-10.10s | ", type_name_of(sym->type));
        } else if (is_struct(sym->type)) {
            const char *tname = type_name_of(sym->type);
            if (tname) {
                static char buf[128] = {'\0'};
                sprintf(buf, "struct %s", tname);
                printf("%-10.10s | ", buf);
            } else {
                printf("%-10.10s | ", "struct");
            }
        } else if (is_enum(sym->type)) {
            const char *tname = type_name_of(sym->type);
            if (tname) {
                static char buf[128] = {'\0'};
                sprintf(buf, "enum %s", tname);
                printf("%-10.10s | ", buf);
            } else {
                printf("%-10.10s | ", "enum");
            }
        } else {
            printf("%-10.10s | ", type_name_of(sym->type));
        }

        /* scope level */
        printf("%5d | ", sym->scope_level);

        /* memory offset */
        if (is_global_var(sym))
            printf("%5s | ",  "*");
        else
            printf("%5d | ",  sym->mem_offset);
        printf("%5d | ",  sym->id);

        /* flags */
        printf("%c", sym->is_extern      ? 'E' : '.');
        printf("%c", sym->is_static      ? 'S' : '.');
        printf("%c", sym->is_defined     ? 'D' : '.');
        printf("%c", sym->is_redefined   ? 'R' : '.');
        printf("%c", sym->is_initialized ? 'I' : '.');
        printf("%c", sym->is_assigned    ? 'A' : '.');
        printf("%c", sym->is_used        ? 'U' : '.');
        printf(" |");

        printf("\n");
    }

    print_horizonal_line('-', COLUMNS);
}

static struct symbol *new_symbol(int kind, const char *name, struct data_type *type,
        int scope_level)
{
    static int next_id = 0;
    struct symbol ini = {0};
    struct symbol *sym = malloc(sizeof(struct symbol));
    *sym = ini;

    sym->kind = kind;
    sym->name = name;
    sym->type = type;
    sym->scope_level = scope_level;
    sym->id = next_id++;

    return sym;
}

static struct symbol *push_symbol(struct symbol_table *table,
        const char *name, int kind, struct data_type *type)
{
    struct symbol *sym = new_symbol(kind, name, type, table->current_scope_level);

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

    case SYM_VAR:
    case SYM_FUNC:
    case SYM_PARAM:
    case SYM_ENUMERATOR:
    case SYM_TYPEDEF:
        return 0;

    case SYM_TAG_STRUCT:
    case SYM_TAG_ENUM:
        return 1;

    case SYM_MEMBER:
        return 2;

    case SYM_LABEL:
        return 3;

    default:
        return -1;
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

static struct symbol *lookup(struct symbol_table *table,
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

static int is_defined_at(struct symbol *sym, int scope)
{
    return sym && sym->scope_level == scope;
}

static void link_type_to_sym(struct data_type *type, struct symbol *sym)
{
    if (sym->kind == SYM_TAG_STRUCT ||
        sym->kind == SYM_TAG_ENUM)
        set_symbol(type, sym);
}

struct symbol *define_symbol(struct symbol_table *table,
        const char *name, int kind, struct data_type *type)
{
    struct symbol *sym = lookup(table, name, kind);
    const int curr_scope = table->current_scope_level;
    struct data_type *defined_type = type;

    if (is_defined_at(sym, curr_scope)) {
        if (is_incomplete(sym->type)) {
            /* the incoming type is discarded then
             * the found incomplete type will be used instead */
            defined_type = sym->type;
        } else {
            sym->is_redefined = 1;
            return sym;
        }
    }

    sym = push_symbol(table, name, kind, defined_type);
    sym->is_defined = 1;
    link_type_to_sym(defined_type, sym);

    return sym;
}

struct symbol *use_symbol(struct symbol_table *table, const char *name, int kind)
{
    struct symbol *sym = lookup(table, name, kind);

    if (!sym) {
        struct data_type *type;
        if (kind == SYM_TAG_STRUCT)
            type = type_struct(name);
        else if (kind == SYM_TAG_ENUM)
            type = type_enum(name);
        else
            type = type_int();
        sym = push_symbol(table, name, kind, type);
        link_type_to_sym(type, sym);
    }

    return sym;
}

static int is_struct_scope(int scope, struct symbol *strct)
{
    if (scope == strct->scope_level + 1)
        return 1;
    else
        return 0;
}

static struct symbol *find_struct_member(struct symbol *strct, const char *member)
{
    struct symbol *sym;

    if (!strct)
        return NULL;

    if (is_incomplete(strct->type))
        return NULL;

    for (sym = strct; sym; sym = sym->next) {
        if (!is_struct_scope(sym->scope_level, strct))
            continue;

        if (match_name(sym, member))
            return sym;

        if (sym->kind == SYM_SCOPE_END)
            break;
    }
    return NULL;
}

struct symbol *use_struct_member_symbol(struct symbol_table *table,
        struct symbol *strct, const char *member)
{
    struct symbol *sym = find_struct_member(strct, member);

    if (!sym) {
        sym = push_symbol(table, member, SYM_MEMBER, type_int());
    }

    return sym;
}

struct symbol *define_case_symbol(struct symbol_table *table, int kind, int case_value)
{
    struct symbol *sym, *case_sym;
    const int cur_swt = table->current_switch_level;

    if (kind != SYM_CASE && kind != SYM_DEFAULT)
        return NULL;

    for (sym = table->tail; sym; sym = sym->prev) {
        if (sym->kind == SYM_CASE && sym->scope_level == cur_swt &&
                sym->mem_offset == case_value) {
            sym->is_redefined = 1;
            return sym;
        }

        if (sym->kind == SYM_DEFAULT && sym->scope_level == cur_swt) {
            sym->is_redefined = 1;
            return sym;
        }

        if (sym->kind == SYM_SWITCH_BEGIN && sym->scope_level == cur_swt)
            break;
    }

    case_sym = push_symbol(table, NULL, kind, NULL);

    case_sym->scope_level = cur_swt;
    case_sym->is_defined = 1;
    case_sym->mem_offset = case_value;

    return case_sym;
}

struct symbol *define_label_symbol(struct symbol_table *table, const char *label)
{
    struct symbol *sym;
    struct symbol *label_sym = NULL;

    for (sym = table->tail; sym; sym = sym->prev) {
        if (match_name(sym, label)) {
            sym->is_redefined = 1;
            return sym;
        }

        /* reached function sym */
        if (is_func(sym))
            break;
    }

    label_sym = push_symbol(table, label, SYM_LABEL, type_int());
    label_sym->is_defined = 1;

    return label_sym;
}

struct symbol *use_label_symbol(struct symbol_table *table, const char *label)
{
    struct symbol *sym;

    for (sym = table->tail; sym; sym = sym->prev) {
        if (match_name(sym, label))
            return sym;

        /* reached function sym */
        if (is_func(sym))
            break;
    }

    return push_symbol(table, label, SYM_LABEL, type_int());
}

struct symbol *define_string_symbol(struct symbol_table *table, const char *str)
{
    struct symbol *sym;
    struct symbol *str_sym = NULL;

    for (sym = table->tail; sym; sym = sym->prev) {
        if (match_name(sym, str))
            return sym;
    }

    str_sym = push_symbol(table, str, SYM_STRING, type_ptr(type_char()));
    str_sym->is_defined = 1;

    return str_sym;
}

struct symbol *find_type_name_symbol(struct symbol_table *table, const char *name)
{
    struct symbol *sym = lookup(table, name, SYM_TYPEDEF);

    if (is_typedef(sym))
        return sym;

    return NULL;
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

static int align_to(int pos, int align)
{
    return ((pos + align - 1) / align) * align;
}

static int is_member_of(const struct symbol *member, const struct symbol *struct_tag)
{
    /* inside of struct is one level upper than struct scope */
    if (!member || !struct_tag)
        return 0;

    return is_member(member) &&
        member->scope_level == struct_tag->scope_level + 1;
}

static int is_struct_end_of(const struct symbol *member, const struct symbol *struct_tag)
{
    /* inside of struct is one level upper than struct scope */
    if (!member || !struct_tag)
        return 0;

    return member->kind == SYM_SCOPE_END &&
        member->scope_level == struct_tag->scope_level + 1;
}

void compute_struct_size(struct symbol_table *table, struct symbol *strc)
{
    struct symbol *sym;
    int total_offset = 0;
    int struct_size = 0;

    if (is_incomplete(strc->type))
        return;

    for (sym = strc; sym; sym = sym->next) {
        if (is_member_of(sym, strc)) {
            const int size  = get_size(sym->type);
            const int align = get_alignment(sym->type);

            total_offset = align_to(total_offset, align);
            sym->mem_offset = total_offset;
            total_offset += size;
        }

        if (is_struct_end_of(sym, strc))
            break;
    }

    struct_size = align_to(total_offset, get_alignment(strc->type));

    set_struct_size(strc->type, struct_size);
    strc->mem_offset = struct_size;
}
