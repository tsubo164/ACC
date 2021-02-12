#ifndef TYPE_H
#define TYPE_H

#include "string_table.h"

enum data_type_kind {
    DATA_TYPE_VOID,
    DATA_TYPE_CHAR,
    DATA_TYPE_INT,
    DATA_TYPE_PTR,
    DATA_TYPE_ARRAY,
    DATA_TYPE_STRUCT,
    DATA_TYPE_ENUM,
    DATA_TYPE_TYPE_NAME
};

struct symbol;

struct data_type {
    int kind;
    int byte_size;
    int alignment;
    int array_len;
    /* TODO may not need this member 'tag' */
    const char *tag;
    struct data_type *ptr_to;
    /* for struct, union, enum tags and typedefs */
    struct symbol *sym;

    char is_const;
};

extern int get_size(const struct data_type *type);
extern int get_alignment(const struct data_type *type);
extern int get_array_length(const struct data_type *type);
extern struct data_type *underlying(const struct data_type *type);
extern struct data_type *original(struct data_type *type);
extern const struct data_type *original_const(const struct data_type *type);
extern struct symbol *symbol_of(const struct data_type *type);
extern const char *tag_of(const struct data_type *type);

extern void set_array_length(struct data_type *type, int len);
extern void set_struct_size(struct data_type *type, int size);
extern void set_symbol(struct data_type *type, struct symbol *sym);
extern void set_const(struct data_type *type, int is_const);

extern struct data_type *promote(struct data_type *t1, struct data_type *t2);
extern int is_incomplete(const struct data_type *type);
extern int is_const(const struct data_type *type);
extern int is_void(const struct data_type *type);
extern int is_char(const struct data_type *type);
extern int is_int(const struct data_type *type);
extern int is_pointer(const struct data_type *type);
extern int is_array(const struct data_type *type);
extern int is_struct(const struct data_type *type);
extern int is_enum(const struct data_type *type);
extern int is_type_name(const struct data_type *type);

extern const char *type_name_of(const struct data_type *type);
extern void print_data_type(const struct data_type *type);

extern struct data_type *type_void();
extern struct data_type *type_char();
extern struct data_type *type_int();
extern struct data_type *type_ptr(struct data_type *base_type);
extern struct data_type *type_array(struct data_type *base_type);
extern struct data_type *type_struct(const char *tag);
extern struct data_type *type_enum(const char *tag);
extern struct data_type *type_type_name(const char *name, struct symbol *type_name);

#endif /* _H */
