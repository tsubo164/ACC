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
    if (type->kind == DATA_TYPE_STRUCT)
        return type->sym->mem_offset;
    return type->byte_size;
}

int get_alignment(const struct data_type *type)
{
    if (type->kind == DATA_TYPE_STRUCT)
        return type->sym->type->alignment;
    return type->alignment;
}

int get_array_length(const struct data_type *type)
{
    if (type->kind == DATA_TYPE_STRUCT)
        return type->sym->type->array_len;
    return type->array_len;
}

int is_incomplete(const struct data_type *type)
{
    if (type->kind == DATA_TYPE_VOID)
        return 1;
    if (type->kind == DATA_TYPE_ENUM && !type->sym->is_defined)
        return 1;
    if (type->kind == DATA_TYPE_STRUCT && !type->sym->is_defined)
        return 1;
    return 0;
}

struct data_type *underlying(const struct data_type *type)
{
    return type->ptr_to;
}

const char *data_type_to_string(const struct data_type *type)
{
    if (!type)
        return "--";

    switch (type->kind) {
    case DATA_TYPE_VOID:   return "void";
    case DATA_TYPE_CHAR:   return "char";
    case DATA_TYPE_INT:    return "int";
    case DATA_TYPE_PTR:    return "ptr";
    case DATA_TYPE_ARRAY:  return "array";
    case DATA_TYPE_STRUCT: return "struct";
    case DATA_TYPE_ENUM:   return "enum";
    default:               return "unknown";
    }
}

void print_data_type(const struct data_type *type)
{
    if (!type)
        return;

    printf("    name:      %s\n", data_type_to_string(type));
    printf("    kind:      %d\n", type->kind);
    printf("    byte_size: %d\n", type->byte_size);
    printf("    array_len: %d\n", type->array_len);
    printf("    tag:       %s\n", type->tag);
    printf("    ptr_to:    %p\n", (void *) type->ptr_to);
}

void print_type_name(const struct data_type *type)
{
    if (type->kind == DATA_TYPE_STRUCT)
        printf("struct %s", type->tag);
    else
        printf("%s", data_type_to_string(type));
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

    /* XXX */
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

struct data_type *type_type_name(const char *name)
{
    struct data_type *type;

    type = malloc(sizeof(struct data_type));
    *type = TYPE_NAME_;
    type->tag = name;

    return type;
}
