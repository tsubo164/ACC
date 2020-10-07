#ifndef TYPE_H
#define TYPE_H

enum data_type_kind {
    DATA_TYPE_VOID,
    DATA_TYPE_CHAR,
    DATA_TYPE_INT,
    DATA_TYPE_PTR,
    DATA_TYPE_ARRAY,
    DATA_TYPE_STRUCT
};

struct data_type {
    int kind;
    int byte_size;
    int alignment;
    int array_len;
    /* XXX */
    const char *tag;
    struct data_type *ptr_to;
};

extern const char *data_type_to_string(const struct data_type *dtype);

extern struct data_type *type_void();
extern struct data_type *type_char();
extern struct data_type *type_int();
extern struct data_type *type_ptr();
extern struct data_type *type_array(struct data_type *base_type, int length);
extern struct data_type *type_struct(const char *tag);

#endif /* _H */
