#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "type.h"
#include "symbol.h"

#define UNKNOWN_ARRAY_LENGTH -1

/*                                                       Sz Al Ln Ul    Sy  */
static struct data_type VOID_     = {DATA_TYPE_VOID,     1, 4, 1, NULL, NULL};
static struct data_type CHAR_     = {DATA_TYPE_CHAR,     1, 1, 1, NULL, NULL};
static struct data_type SHORT_    = {DATA_TYPE_SHORT,    2, 2, 1, NULL, NULL};
static struct data_type INT_      = {DATA_TYPE_INT,      4, 4, 1, NULL, NULL};
static struct data_type LONG_     = {DATA_TYPE_LONG,     8, 8, 1, NULL, NULL};
static struct data_type POINTER_  = {DATA_TYPE_POINTER,  8, 8, 1, NULL, NULL};
static struct data_type ARRAY_    = {DATA_TYPE_ARRAY,    0, 0, UNKNOWN_ARRAY_LENGTH, NULL, NULL};
static struct data_type STRUCT_   = {DATA_TYPE_STRUCT,   1, 1, 1, NULL, NULL};
static struct data_type UNION_    = {DATA_TYPE_UNION,    1, 1, 1, NULL, NULL};
static struct data_type ENUM_     = {DATA_TYPE_ENUM,     4, 4, 1, NULL, NULL};
static struct data_type FUNCTION_ = {DATA_TYPE_FUNCTION, 4, 4, 1, NULL, NULL};

int get_size(const struct data_type *type)
{
    if (is_array(type))
        return get_array_length(type) * get_size(underlying(type));
    return type->byte_size;
}

int get_alignment(const struct data_type *type)
{
    return type->alignment;
}

int get_array_length(const struct data_type *type)
{
    return type->array_len;
}

struct data_type *underlying(const struct data_type *type)
{
    if (!type)
        return NULL;
    return type->base;
}

struct symbol *symbol_of(const struct data_type *type)
{
    return type->sym;
}

void set_array_length(struct data_type *type, int len)
{
    if (!is_array(type))
        return;
    type->array_len = len;
}

void set_struct_size(struct data_type *type, int size)
{
    if (!is_struct(type))
        return;
    type->byte_size = size;
}

void set_struct_align(struct data_type *type, int align)
{
    if (!is_struct(type))
        return;
    type->alignment = align;
}

void set_union_size(struct data_type *type, int size)
{
    if (!is_union(type))
        return;
    type->byte_size = size;
}

void set_union_align(struct data_type *type, int align)
{
    if (!is_union(type))
        return;
    type->alignment = align;
}

void set_symbol(struct data_type *type, struct symbol *sym)
{
    type->sym = sym;
}

void set_const(struct data_type *type, int is_const)
{
    type->is_const = is_const;
}

void set_unsigned(struct data_type *type, int is_unsigned)
{
    type->is_unsigned = is_unsigned;
}

struct data_type *promote(struct data_type *t1, struct data_type *t2)
{
    if (!t1 && !t2)
        return type_void();
    if (!t1)
        return t2;
    if (!t2)
        return t1;

    if (t1->kind == t2->kind) {
        if (is_unsigned(t1))
            return t1;
        else
            return t2;
    }

    if (t1->kind > t2->kind)
        return t1;
    else
        return t2;
}

int has_unkown_array_length(const struct data_type *type)
{
    return get_array_length(type) == UNKNOWN_ARRAY_LENGTH;
}

int has_typedef_name(const struct data_type *type)
{
    return type->alias != NULL;
}

static int is_compatible_underlying(const struct data_type *t1, const struct data_type *t2)
{
    if (!t1 || !t2)
        return 0;

    if ((is_char(t1) && is_char(t2)) ||
        (is_short(t1) && is_short(t2)) ||
        (is_int(t1) && is_int(t2)) ||
        (is_long(t1) && is_long(t2)))
        return 1;

    if (is_struct(t1) && is_struct(t2))
        return is_compatible(t1, t2);

    if (is_union(t1) && is_union(t2))
        return is_compatible(t1, t2);

    if (is_pointer(t1) && is_pointer(t2))
        return is_compatible_underlying(underlying(t1), underlying(t2));

    if (is_function(t1) && is_function(t2)) {
        /* checking return type only for now */
        return is_compatible(underlying(t1), underlying(t2));
    }

    return 0;
}

int is_compatible(const struct data_type *t1, const struct data_type *t2)
{
    if (!t1 || !t2)
        return 0;

    if (is_integer(t1) && is_integer(t2))
        return 1;

    if (is_struct(t1) && is_struct(t2))
        return symbol_of(t1) == symbol_of(t2);

    if (is_union(t1) && is_union(t2))
        return symbol_of(t1) == symbol_of(t2);

    /* 6.3.2.3 Pointers void * <-> T * */
    if (is_pointer(t1) && is_pointer(t2) && is_void(underlying(t2)))
        return 1;
    if (is_pointer(t1) && is_void(underlying(t1)) && is_pointer(t2))
        return 1;

    if (is_pointer(t1) && is_pointer(t2))
        return is_compatible_underlying(underlying(t1), underlying(t2));

    return 0;
}

int is_incomplete(const struct data_type *type)
{
    if (is_void(type))
        return 1;
    if (is_struct(type) && !type->sym->is_defined)
        return 1;
    if (is_union(type) && !type->sym->is_defined)
        return 1;
    if (is_enum(type) && !type->sym->is_defined)
        return 1;
    return 0;
}

int is_integer(const struct data_type *type)
{
    if (is_char(type) ||
        is_short(type) ||
        is_int(type) ||
        is_long(type))
        return 1;
    if (is_enum(type))
        return 1;
    return 0;
}

int is_const(const struct data_type *type)
{
    return type->is_const;
}

int is_unsigned(const struct data_type *type)
{
    return type->is_unsigned;
}

int is_void(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_VOID;
}

int is_char(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_CHAR;
}

int is_short(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_SHORT;
}

int is_int(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_INT;
}

int is_long(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_LONG;
}

int is_pointer(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_POINTER;
}

int is_array(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_ARRAY;
}

int is_struct(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_STRUCT;
}

int is_union(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_UNION;
}

int is_enum(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_ENUM;
}

int is_struct_or_union(const struct data_type *type)
{
    return is_struct(type) || is_union(type);
}

int is_function(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_FUNCTION;
}

static char *make_type_name_(const struct data_type *type, char *buf)
{
    char *p = buf;
    int n = 0;

    if (!type)
        return NULL;

    if (is_const(type)) {
        sprintf(p, "const %n", &n);
        p += n;
    }

    if (is_void(type)) {
        sprintf(p, "void %n", &n);
        p += n;
    }
    else if (is_char(type)) {
        sprintf(p, "char %n", &n);
        p += n;
    }
    else if (is_short(type)) {
        sprintf(p, "short %n", &n);
        p += n;
    }
    else if (is_int(type)) {
        sprintf(p, "int %n", &n);
        p += n;
    }
    else if (is_long(type)) {
        sprintf(p, "long %n", &n);
        p += n;
    }
    else if (is_pointer(type)) {
        /* XXX need improvement to orders to write */
        if (is_array(underlying(type)) || is_function(underlying(type))) {
            sprintf(p, "(*)%n", &n);
            p += n;
            p = make_type_name_(underlying(type), p);
        }
        else {
            p = make_type_name_(underlying(type), p);
            sprintf(p, "*%n", &n);
            p += n;
        }
    }
    else if (is_array(type)) {
        const struct data_type *t;

        /* print underlying type first */
        for (t = type; is_array(t); t = underlying(t))
            ;
        p = make_type_name_(t, p);

        /* print array lengths */
        for (t = type; is_array(t); t = underlying(t)) {
            sprintf(p, "[%d]%n", get_array_length(t), &n);
            p += n;
        }
    }
    else if (is_struct(type)) {
        sprintf(p, "struct %s %n", symbol_of(type)->name, &n);
        p += n;
    }
    else if (is_union(type)) {
        sprintf(p, "union %s %n", symbol_of(type)->name, &n);
        p += n;
    }
    else if (is_function(type)) {
        p = make_type_name_(underlying(type), p);
        sprintf(p++, "(");
        {
            const struct symbol *s = first_param(symbol_of(type));
            if (!s) {
                sprintf(p, "void%n", &n);
                p += n;
            }
            else {
                int i = 0;
                for (; s; s = next_param(s)) {
                    if (*(p-1) == ' ')
                        p--;
                    if (i++ > 0) {
                        *p++ = ',';
                        *p++ = ' ';
                    }

                    if (is_ellipsis(s)) {
                        sprintf(p, "...%n", &n);
                        p += n;
                        break;
                    }
                    else {
                        p = make_type_name_(s->type, p);
                    }
                }
            }
        }
        if (*(p-1) == ' ')
            p--;
        sprintf(p++, ")");
    }

    return p;
}

void make_type_name(const struct data_type *type, char *buf)
{
    char *p;

    if (!type)
        return;

    p = make_type_name_(type, buf);
    /* trim trailing space */
    if (p != buf && *(p-1) == ' ')
        *(p-1) = '\0';
}

void print_data_type(const struct data_type *type)
{
    char type_name[128] = {'\0'};

    if (!type)
        return;

    make_type_name(type, type_name);
    printf("    name:      %s\n", type_name);
    printf("    kind:      %d\n", type->kind);
    printf("    byte_size: %d\n", type->byte_size);
    printf("    array_len: %d\n", type->array_len);
    printf("    base:      %p\n", (void *) type->base);
    printf("    sym:       %p\n", (void *) type->sym);
}

void copy_data_type(struct data_type *dst, const struct data_type *src)
{
    if (!dst || !src || dst == src)
        return;
    *dst = *src;
}

void convert_array_to_pointer(struct data_type *type)
{
    if (is_array(type)) {
        struct data_type *base = underlying(type);
        const int is_const = type->is_const;

        convert_array_to_pointer(base);
        copy_data_type(type, &POINTER_);

        type->base = base;
        type->is_const = is_const;
    }
}

static struct data_type *clone(const struct data_type *orig)
{
    struct data_type *type = malloc(sizeof(struct data_type));
    *type = *orig;
    return type;
}

struct data_type *type_void(void)
{
    return clone(&VOID_);
}

struct data_type *type_char(void)
{
    return clone(&CHAR_);
}

struct data_type *type_short(void)
{
    return clone(&SHORT_);
}

struct data_type *type_int(void)
{
    return clone(&INT_);
}

struct data_type *type_long(void)
{
    return clone(&LONG_);
}

struct data_type *type_pointer(struct data_type *base_type)
{
    struct data_type *type = clone(&POINTER_);
    type->base = base_type;
    return type;
}

struct data_type *type_array(struct data_type *base_type)
{
    struct data_type *type = clone(&ARRAY_);

    type->base = base_type;
    type->byte_size = base_type->byte_size;
    type->alignment = base_type->alignment;
    /* type->array_len will be computed in later phase */

    return type;
}

struct data_type *type_struct(void)
{
    struct data_type *type = clone(&STRUCT_);
    return type;
}

struct data_type *type_union(void)
{
    struct data_type *type = clone(&UNION_);
    return type;
}

struct data_type *type_enum(void)
{
    struct data_type *type = clone(&ENUM_);
    return type;
}

struct data_type *type_type_name(struct symbol *type_name)
{
    struct data_type *type = clone(type_name->type);
    type->alias = type_name;
    return type;
}

struct data_type *type_function(struct data_type *return_type)
{
    struct data_type *type = clone(&FUNCTION_);
    type->base = return_type;
    return type;
}
