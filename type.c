#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "type.h"
#include "symbol.h"

static struct data_type VOID_    = {DATA_TYPE_VOID,   1, 4, 1, NULL, NULL};
static struct data_type CHAR_    = {DATA_TYPE_CHAR,   1, 4, 1, NULL, NULL};
static struct data_type INT_     = {DATA_TYPE_INT,    4, 4, 1, NULL, NULL};
static struct data_type PTR_     = {DATA_TYPE_PTR,    8, 8, 1, NULL, NULL};
static struct data_type ARRAY_   = {DATA_TYPE_ARRAY,  0, 0, 0, NULL, NULL};
static struct data_type STRUCT_  = {DATA_TYPE_STRUCT, 0, 4, 1, NULL, NULL};
static struct data_type ENUM_    = {DATA_TYPE_ENUM,   4, 4, 1, NULL, NULL};
static struct data_type TYPE_NAME_ = {DATA_TYPE_TYPE_NAME, 0, 4, 1, NULL, NULL};

int get_size(const struct data_type *type)
{
    if (is_struct(type))
        return type->sym->type->byte_size;
    if (is_type_name(type))
        return get_size(type->sym->type);
    return type->byte_size;
}

int get_alignment(const struct data_type *type)
{
    if (is_struct(type))
        return type->sym->type->alignment;
    if (is_type_name(type))
        return get_alignment(type->sym->type);
    return type->alignment;
}

int get_array_length(const struct data_type *type)
{
    if (is_struct(type))
        return type->sym->type->array_len;
    if (is_type_name(type))
        return get_array_length(type->sym->type);
    return type->array_len;
}

struct data_type *underlying(const struct data_type *type)
{
    return type->ptr_to;
}

struct data_type *original(struct data_type *type)
{
    if (is_type_name(type))
        return type->sym->type;
    return type;
}

const struct data_type *original_const(const struct data_type *type)
{
    if (is_type_name(type))
        return type->sym->type;
    return type;
}

struct symbol *symbol_of(const struct data_type *type)
{
    if (is_type_name(type))
        return symbol_of(original_const(type));
    return type->sym;
}

const char *tag_of(const struct data_type *type)
{
    return type->tag;
}

void set_struct_size(struct data_type *type, int size)
{
    if (!is_struct(type))
        return;
    type->byte_size = size;
}

void set_symbol(struct data_type *type, struct symbol *sym)
{
    type->sym = sym;
}

const struct data_type *promote(
        const struct data_type *t1, const struct data_type *t2)
{
    if (!t1 && !t2)
        return type_void();
    if (!t1)
        return t2;
    if (!t2)
        return t1;

    if (t1->kind > t2->kind)
        return t1;
    else
        return t2;
}

int is_incomplete(const struct data_type *type)
{
    if (is_void(type))
        return 1;
    if (is_enum(type) && !type->sym->is_defined)
        return 1;
    if (is_struct(type) && !type->sym->is_defined)
        return 1;
    return 0;
}

int is_void(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_VOID;
}

int is_char(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_CHAR;
}

int is_int(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_INT;
}

int is_array(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_ARRAY;
}

int is_struct(const struct data_type *type)
{
    if (!type)
        return 0;
    if (type->kind == DATA_TYPE_STRUCT)
        return 1;
    if (is_type_name(type))
        return is_struct(type->sym->type);
    return 0;
}

int is_enum(const struct data_type *type)
{
    if (!type)
        return 0;
    if (type->kind == DATA_TYPE_ENUM)
        return 1;
    if (is_type_name(type))
        return is_enum(type->sym->type);
    return 0;
}

int is_type_name(const struct data_type *type)
{
    if (!type)
        return 0;
    return type->kind == DATA_TYPE_TYPE_NAME;
}

const char *type_name_of(const struct data_type *type)
{
    if (!type)
        return "--";

    switch (type->kind) {
    case DATA_TYPE_VOID:   return "void";
    case DATA_TYPE_CHAR:   return "char";
    case DATA_TYPE_INT:    return "int";
    case DATA_TYPE_PTR:    return "ptr";
    case DATA_TYPE_ARRAY:  return "array";
    case DATA_TYPE_STRUCT: return type->tag;
    case DATA_TYPE_ENUM:   return type->tag;
    case DATA_TYPE_TYPE_NAME: return type->tag;
    default:               return "unknown";
    }
}

void print_data_type(const struct data_type *type)
{
    if (!type)
        return;

    printf("    name:      ");
    if (is_struct(type))
        printf("struct ");
    printf("%s\n", type_name_of(type));

    printf("    kind:      %d\n", type->kind);
    printf("    byte_size: %d\n", type->byte_size);
    printf("    array_len: %d\n", type->array_len);
    printf("    tag:       %s\n", type->tag);
    printf("    ptr_to:    %p\n", (void *) type->ptr_to);
    printf("    sym:       %p\n", (void *) type->sym);
}

struct data_type *type_void()
{
    return &VOID_;
}

struct data_type *type_char()
{
    return &CHAR_;
}

struct data_type *type_int()
{
    return &INT_;
}

struct data_type *type_ptr(struct data_type *base_type)
{
    struct data_type *type;

    type = malloc(sizeof(struct data_type));
    *type = PTR_;
    type->ptr_to = base_type;

    return type;
}

struct data_type *type_array(struct data_type *base_type, int length)
{
    struct data_type *type;

    type = malloc(sizeof(struct data_type));
    *type = ARRAY_;
    type->ptr_to = base_type;

    type->byte_size = base_type->byte_size;
    type->alignment = base_type->alignment;
    type->array_len = length;

    return type;
}

struct data_type *type_struct(const char *tag)
{
    struct data_type *type;

    type = malloc(sizeof(struct data_type));
    *type = STRUCT_;
    type->tag = tag;

    return type;
}

struct data_type *type_enum(const char *tag)
{
    struct data_type *type;

    type = malloc(sizeof(struct data_type));
    *type = ENUM_;
    type->tag = tag;

    return type;
}

struct data_type *type_type_name(const char *name, struct symbol *type_name)
{
    struct data_type *type;

    type = malloc(sizeof(struct data_type));
    *type = TYPE_NAME_;
    type->tag = name;
    type->sym = type_name;

    return type;
}
