#include <stdlib.h>
#include <string.h>
#include "type.h"

static struct data_type VOID_    = {DATA_TYPE_VOID,   0, 0, 1, NULL};
static struct data_type CHAR_    = {DATA_TYPE_CHAR,   1, 4, 1, NULL};
static struct data_type INT_     = {DATA_TYPE_INT,    4, 4, 1, NULL};
static struct data_type PTR_     = {DATA_TYPE_PTR,    8, 8, 1, NULL};
static struct data_type ARRAY_   = {DATA_TYPE_ARRAY,  0, 0, 0, NULL};
static struct data_type STRUCT_  = {DATA_TYPE_STRUCT, 0, 0, 1, NULL};

const char *data_type_to_string(const struct data_type *dtype)
{
    switch (dtype->kind) {
    case DATA_TYPE_VOID:  return "void";
    case DATA_TYPE_CHAR:  return "char";
    case DATA_TYPE_INT:   return "int";
    case DATA_TYPE_PTR:   return "ptr";
    case DATA_TYPE_ARRAY: return "array";
    default:              return "unknown";
    }
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

struct data_type *type_ptr()
{
    struct data_type *dtype;

    dtype = malloc(sizeof(struct data_type));
    *dtype = PTR_;

    return dtype;
}

struct data_type *type_array(struct data_type *base_type, int length)
{
    struct data_type *dtype;

    dtype = malloc(sizeof(struct data_type));
    *dtype = ARRAY_;
    dtype->ptr_to = base_type;

    /* XXX */
    dtype->byte_size = base_type->byte_size;
    dtype->alignment = base_type->alignment;
    dtype->array_len = length;

    return dtype;
}

struct data_type *type_struct(const char *tag)
{
    struct data_type *dtype;

    dtype = malloc(sizeof(struct data_type));
    *dtype = STRUCT_;

    {
        /* XXX */
        char *t = (char *) malloc(sizeof(char) * (strlen(tag)+1));
        strcpy(t, tag);
        dtype->tag = t;
    }

    return dtype;
}

