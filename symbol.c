#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "symbol.h"
#include "esc_seq.h"

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

int is_func_prototype(const struct symbol *sym)
{
    return sym && sym->kind == SYM_FUNC && !sym->is_defined;
}

int is_param(const struct symbol *sym)
{
    return sym && sym->kind == SYM_PARAM;
}

int is_struct_tag(const struct symbol *sym)
{
    return sym && sym->kind == SYM_TAG_STRUCT;
}

int is_union_tag(const struct symbol *sym)
{
    return sym && sym->kind == SYM_TAG_UNION;
}

int is_member(const struct symbol *sym)
{
    return sym && sym->kind == SYM_MEMBER;
}

int is_bitfield(const struct symbol *sym)
{
    return is_member(sym) && sym->is_bitfield;
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

int is_ellipsis(const struct symbol *sym)
{
    return sym && sym->kind == SYM_ELLIPSIS;
}

int is_string_literal(const struct symbol *sym)
{
    return sym && sym->kind == SYM_STRING;
}

int is_fpnum_literal(const struct symbol *sym)
{
    return sym && sym->kind == SYM_FPNUM;
}

int is_variadic(const struct symbol *sym)
{
    return is_func(sym) && sym->is_variadic;
}

int is_builtin(const struct symbol *sym)
{
    return sym && sym->is_builtin;
}

int is_origin(const struct symbol *sym)
{
    return sym && !sym->orig;
}

int has_origin(const struct symbol *sym)
{
    return !is_origin(sym);
}

static int is_scope_begin(const struct symbol *sym)
{
    return sym && sym->kind == SYM_SCOPE_BEGIN;
}

struct symbol_table *new_symbol_table(void)
{
    struct symbol_table *table;
    table = malloc(sizeof(struct symbol_table));

    table->head = NULL;
    table->tail = NULL;
    /* 0 means global scope */
    table->current_scope_level = 0;
    /* switch scope is independent of current scope */
    table->current_switch_level = 0;

    return table;
}

static void free_list(struct symbol *sym)
{
    struct symbol *s = sym, *tmp;

    while (s) {
        tmp = s->next;
        free(s);
        s = tmp;
    }
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
    case SYM_FPNUM: return "SYM_FPNUM";
    case SYM_VAR: return "SYM_VAR";
    case SYM_FUNC: return "SYM_FUNC";
    case SYM_PARAM: return "SYM_PARAM";
    case SYM_MEMBER: return "SYM_MEMBER";
    case SYM_ENUMERATOR: return "SYM_ENUMERATOR";
    case SYM_TAG_STRUCT: return "SYM_TAG_STRUCT";
    case SYM_TAG_UNION: return "SYM_TAG_UNION";
    case SYM_TAG_ENUM: return "SYM_TAG_ENUM";
    case SYM_TYPEDEF: return "SYM_TYPEDEF";
    case SYM_ELLIPSIS: return "SYM_ELLIPSIS";
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
    const int COLUMNS = 101;
    const struct symbol *sym;

    print_horizonal_line('-', COLUMNS);

    printf("|");
    printf("%15s | ", "name");
    printf("%20s | ", "kind");
    printf("%15s | ", "type");
    printf("%5s | ", "scope");
    printf("%5s| ", "offset");
    printf("%5s | ", "id");
    printf("%5s | ", "orig");
    printf("%7s | ", "ESDRIAU");
    printf("\n");

    print_horizonal_line('=', COLUMNS);

    for (sym = table->head; sym; sym = sym->next) {
        printf("|");
        /* symbol name */
        if (sym->kind == SYM_STRING) {
            char buf[1024] = {'\0'};
            make_string_literal(sym->name, buf, sizeof(buf)/sizeof(buf[0]));
            printf("%15.15s | ", buf);
        } else {
            printf("%15.15s | ", sym->name ? sym->name : "--");
        }

        /* symbol kind */
        printf("%-20s | ",  symbol_to_string(sym));

        /* type */
        if (sym->type) {
            char buf[256] = {'\0'};
            make_type_name(sym->type, buf);
            printf("%-15.15s | ", buf);
        } else {
            printf("%-15.15s | ", "--");
        }

        /* scope level */
        printf("%5d | ", sym->scope_level);

        /* memory offset */
        if (is_global_var(sym))
            printf("%5s | ",  "*");
        else if (is_bitfield(sym))
            printf("%dw%db%dm | ", sym->bit_width, sym->bit_offset, sym->mem_offset);
        else
            printf("%5d | ",  sym->mem_offset);
        /* id */
        printf("%5d | ",  sym->id);
        /* orig */
        if (has_origin(sym))
            printf("%5d | ",  sym->orig->id);
        else
            printf("%5s | ",  "--");

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

static int starts_with(const char *str, const char *prefix)
{
    const char *s = str, *p = prefix;

    if (!s || !p)
        return 0;

    while (*p != '\0') {
        if (*s == '\0')
            return 0;
        if (*s != *p)
            return 0;
        s++;
        p++;
    }
    return 1;
}

static struct symbol *new_symbol(int kind, const char *name, struct data_type *type,
        int scope_level)
{
    static int next_id = 0;
    struct symbol *sym = calloc(1, sizeof(struct symbol));

    sym->kind = kind;
    sym->name = name;
    sym->type = type;
    sym->scope_level = scope_level;
    sym->id = next_id++;

    if (starts_with(sym->name, "__builtin_"))
        sym->is_builtin = 1;

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
    case SYM_TAG_UNION:
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
    int lv = table->current_scope_level;

    for (sym = table->tail; sym; sym = sym->prev) {
        /* step down one level */
        if (is_scope_begin(sym)) {
            lv = sym->scope_level < lv ? sym->scope_level : lv;
            continue;
        }

        if (sym->scope_level > lv)
            continue;

        if (match_name(sym, name) && match_namespace(sym, kind))
            return sym;
    }

    return NULL;
}

static struct symbol *lookup_current(struct symbol_table *table,
        const char *name, enum symbol_kind kind)
{
    struct symbol *sym;
    const int lv = table->current_scope_level;

    for (sym = table->tail; sym; sym = sym->prev) {
        /* reached the beginning of current scope */
        if (is_scope_begin(sym) && sym->scope_level < lv)
            break;

        if (sym->scope_level > lv)
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
    if (is_struct_tag(sym) ||
        is_union_tag(sym) ||
        is_enum_tag(sym) ||
        is_func(sym))
        set_symbol(type, sym);
}

static struct symbol *get_origin(struct symbol *sym)
{
    if (has_origin(sym))
        return sym->orig;
    else
        return sym;
}

struct symbol *define_symbol(struct symbol_table *table,
        const char *name, int kind, struct data_type *type)
{
    struct symbol *sym = NULL, *orig = NULL;
    struct symbol *new_sym = NULL;
    const int curr_scope = table->current_scope_level;
    struct data_type *defined_type = type;
    int already_defined = 0;

    sym = lookup_current(table, name, kind);

    /* TODO may not need SYM_FUNC check when making define functions
     * for each symbol kind */
    if (is_func_prototype(sym) && kind == SYM_FUNC) {
        /* the pointer to original declaration/definition */
        orig = get_origin(sym);
    }
    else if (is_defined_at(sym, curr_scope)) {
        if (is_incomplete(sym->type)) {
            /* the incoming type is discarded then
             * the found incomplete type will be used instead */
            defined_type = sym->type;
        } else {
            already_defined = 1;
            orig = get_origin(sym);
        }
    }

    new_sym = push_symbol(table, name, kind, defined_type);

    new_sym->is_defined = !already_defined;
    new_sym->is_redefined = already_defined;
    new_sym->orig = orig;
    link_type_to_sym(defined_type, new_sym);

    return new_sym;
}

struct symbol *use_symbol(struct symbol_table *table, const char *name, int kind)
{
    struct symbol *sym = lookup(table, name, kind);

    if (!sym) {
        struct data_type *type;
        if (kind == SYM_TAG_STRUCT)
            type = type_struct();
        else if (kind == SYM_TAG_UNION)
            type = type_union();
        else if (kind == SYM_TAG_ENUM)
            type = type_enum();
        else
            type = type_int();
        sym = push_symbol(table, name, kind, type);
        link_type_to_sym(type, sym);
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
        if (is_case(sym) &&
            kind == SYM_CASE &&
            sym->scope_level == cur_swt &&
            sym->mem_offset == case_value) {
            sym->is_redefined = 1;
            return sym;
        }

        if (is_default(sym) &&
            kind == SYM_DEFAULT &&
            sym->scope_level == cur_swt) {
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
    struct symbol *sym = NULL, *orig = NULL;
    struct symbol *new_sym = NULL;
    int already_defined = 0;

    for (sym = table->tail; sym; sym = sym->prev) {
        if (match_name(sym, label)) {
            if (!sym->is_defined) {
                /* label used first, then defined */
                sym->is_defined = 1;
                return sym;
            } else {
                /* re-defined */
                already_defined = 1;
                orig = get_origin(sym);
                break;
            }
        }

        /* reached function sym */
        if (is_func(sym))
            break;
    }

    new_sym = push_symbol(table, label, SYM_LABEL, type_int());
    new_sym->is_defined = !already_defined;
    new_sym->is_redefined = already_defined;
    new_sym->orig = orig;

    return new_sym;
}

struct symbol *use_label_symbol(struct symbol_table *table, const char *label)
{
    struct symbol *sym;

    for (sym = table->tail; sym; sym = sym->prev) {
        if (match_name(sym, label) && !sym->is_redefined)
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
    struct data_type *str_type = NULL;

    for (sym = table->tail; sym; sym = sym->prev)
        if (match_name(sym, str) && is_string_literal(sym))
            return sym;

    str_type = type_array(type_char());
    set_array_length(str_type, strlen(str) + 1);

    str_sym = push_symbol(table, str, SYM_STRING, str_type);
    str_sym->is_defined = 1;

    return str_sym;
}

struct symbol *define_fpnum_symbol(struct symbol_table *table, const char *str)
{
    struct symbol *sym;
    struct symbol *str_sym = NULL;
    struct data_type *fp_type = NULL;

    for (sym = table->tail; sym; sym = sym->prev)
        if (match_name(sym, str) && is_fpnum_literal(sym))
            return sym;

    fp_type = type_double();

    str_sym = push_symbol(table, str, SYM_FPNUM, fp_type);
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

struct symbol *define_ellipsis_symbol(struct symbol_table *table)
{
    return push_symbol(table, "...", SYM_ELLIPSIS, type_void());
}

void symbol_scope_begin(struct symbol_table *table)
{
    push_symbol(table, NULL, SYM_SCOPE_BEGIN, NULL);
    table->current_scope_level++;
}

void symbol_scope_end(struct symbol_table *table)
{
    table->current_scope_level--;
    push_symbol(table, NULL, SYM_SCOPE_END, NULL);
}

void symbol_switch_begin(struct symbol_table *table)
{
    struct symbol *sym;

    table->current_switch_level++;
    sym = push_symbol(table, NULL, SYM_SWITCH_BEGIN, NULL);
    sym->scope_level = table->current_switch_level;
}

void symbol_switch_end(struct symbol_table *table)
{
    struct symbol *sym;

    sym = push_symbol(table, NULL, SYM_SWITCH_END, NULL);
    sym->scope_level = table->current_switch_level;
    table->current_switch_level--;
}
