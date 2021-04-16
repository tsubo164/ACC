#ifndef TYPE_H
#define TYPE_H

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

extern int get_size(const struct data_type *type);
extern int get_alignment(const struct data_type *type);
extern int get_array_length(const struct data_type *type);
extern struct data_type *underlying(const struct data_type *type);
extern struct symbol *symbol_of(const struct data_type *type);

extern void set_array_length(struct data_type *type, int len);
extern void set_struct_size(struct data_type *type, int size);
extern void set_symbol(struct data_type *type, struct symbol *sym);
extern void set_const(struct data_type *type, int is_const);
extern void set_unsigned(struct data_type *type, int is_unsigned);

extern struct data_type *promote(struct data_type *t1, struct data_type *t2);
extern int has_unkown_array_length(const struct data_type *type);
extern int has_typedef_name(const struct data_type *type);
extern int is_compatible(const struct data_type *t1, const struct data_type *t2);
extern int is_incomplete(const struct data_type *type);
extern int is_integer(const struct data_type *type);
extern int is_const(const struct data_type *type);
extern int is_unsigned(const struct data_type *type);
extern int is_void(const struct data_type *type);
extern int is_char(const struct data_type *type);
extern int is_short(const struct data_type *type);
extern int is_int(const struct data_type *type);
extern int is_long(const struct data_type *type);
extern int is_pointer(const struct data_type *type);
extern int is_array(const struct data_type *type);
extern int is_struct(const struct data_type *type);
extern int is_enum(const struct data_type *type);

extern void make_type_name(const struct data_type *type, char *buf);
extern const char *type_name_of(const struct data_type *type);
extern void print_data_type(const struct data_type *type);
extern void copy_data_type(struct data_type *dst, const struct data_type *src);

extern struct data_type *type_void(void);
extern struct data_type *type_char(void);
extern struct data_type *type_short(void);
extern struct data_type *type_int(void);
extern struct data_type *type_long(void);
extern struct data_type *type_pointer(struct data_type *base_type);
extern struct data_type *type_array(struct data_type *base_type);
extern struct data_type *type_struct(void);
extern struct data_type *type_enum(void);
extern struct data_type *type_type_name(struct symbol *type_name);

#endif /* _H */
