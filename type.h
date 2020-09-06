#ifndef TYPE_H
#define TYPE_H

enum data_type_kind {
    DATA_TYPE_VOID,
    DATA_TYPE_INT,
    DATA_TYPE_PTR
};

struct data_type {
    int kind;
    int byte_size;
    struct data_type *ptr_to;
};

extern struct data_type *type_void();
extern struct data_type *type_int();
extern struct data_type *type_ptr();

#endif /* _H */
