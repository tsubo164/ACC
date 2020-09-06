#include "type.h"

static struct data_type VOID_ = {DATA_TYPE_VOID, 0};
static struct data_type INT_  = {DATA_TYPE_INT,  4};
static struct data_type PTR_  = {DATA_TYPE_PTR,  8};

struct data_type *type_void()
{
    return &VOID_;
}

struct data_type *type_int()
{
    return &INT_;
}

struct data_type *type_ptr()
{
    return &PTR_;
}
