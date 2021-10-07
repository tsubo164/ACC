/*
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
*/

#include <stdlib.h>
#define UNKNOWN_ARRAY_LENGTH -1

enum data_type_kind {
    DATA_TYPE_VOID,
    DATA_TYPE_CHAR,
    DATA_TYPE_SHORT,
    DATA_TYPE_INT,
    DATA_TYPE_LONG,
    DATA_TYPE_POINTER,
    DATA_TYPE_ARRAY,
    DATA_TYPE_STRUCT,
    DATA_TYPE_ENUM
};

struct symbol;

struct data_type {
    int kind;
    int byte_size;
    int alignment;
    int array_len;
    struct data_type *ptr_to;
    /* struct, union, enum tags */
    struct symbol *sym;
    /* typedefs */
    struct symbol *alias;
    char is_const;
    char is_unsigned;
};

struct data_type VOID_    = {DATA_TYPE_VOID,    1, 4, 1, NULL, NULL};
struct data_type ARRAY_   = {DATA_TYPE_ARRAY,   0, 0, UNKNOWN_ARRAY_LENGTH, NULL, NULL};

int main()
{
    int a = DATA_TYPE_INT;
    return a;
}
