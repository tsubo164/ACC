#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"

enum symbol_kind {
    SYM_SCOPE_BEGIN,
    SYM_SCOPE_END,
    SYM_VAR,
    SYM_GLOBAL_VAR,
    SYM_FUNC,
    SYM_PARAM
};

struct symbol {
    char *name;
    int kind;
    int mem_offset;

    const struct data_type *dtype;
    /*
    int local_var_id;
    */
    int scope_level;
};

#define SYMBOL_TABLE_SIZE 128
struct symbol_table {
    struct symbol data[SYMBOL_TABLE_SIZE];
    int symbol_count;

    /*
    int local_var_id;
    */
    int current_scope_level;
};

extern void init_symbol_table(struct symbol_table *table);

extern struct symbol *lookup_symbol(struct symbol_table *table,
        const char *name, enum symbol_kind kind);

extern struct symbol *insert_symbol(struct symbol_table *table,
        const char *name, enum symbol_kind kind);

extern int symbol_scope_begin(struct symbol_table *table);
extern int symbol_scope_end(struct symbol_table *table);

/* XXX */
extern struct symbol_table *new_symbol_table();
extern void free_symbol_table(struct symbol_table *table);

/* XXX semantics? this includes mem offset logic that depends on architure */
extern int symbol_assign_local_storage(struct symbol_table *table);

#endif /* _H */
