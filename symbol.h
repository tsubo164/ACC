#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"

enum symbol_flag {
    IS_DECLARED    = 1 << 0,
    IS_DEFINED     = 1 << 1,
    IS_REDEFINED   = 1 << 2,
    IS_INITIALIZED = 1 << 3,
    IS_USED        = 1 << 4
};

enum symbol_kind {
    SYM_SCOPE_BEGIN,
    SYM_SCOPE_END,
    SYM_VAR,
    SYM_FUNC,
    SYM_PARAM,
    SYM_STRUCT
};

struct symbol {
    char *name;
    int kind;
    int flag;
    int mem_offset;

    struct data_type *type;
    int scope_level;

    /* TODO improve this */
    long file_pos;
};

#define SYMBOL_TABLE_SIZE 128
struct symbol_table {
    struct symbol data[SYMBOL_TABLE_SIZE];
    int symbol_count;
    int current_scope_level;
};

/* symbol */
extern int is_global_var(const struct symbol *sym);
extern int is_local_var(const struct symbol *sym);
extern int is_param(const struct symbol *sym);
extern void symbol_flag_on(struct symbol *sym, int flag);
extern int symbol_flag_is_on(const struct symbol *sym, int flag);

/* symbol table */
extern struct symbol_table *new_symbol_table();
extern void free_symbol_table(struct symbol_table *table);
extern void init_symbol_table(struct symbol_table *table);

extern struct symbol *lookup_symbol(struct symbol_table *table,
        const char *name, enum symbol_kind kind);
extern struct symbol *insert_symbol(struct symbol_table *table,
        const char *name, enum symbol_kind kind);
extern struct symbol *use_symbol(struct symbol_table *table, const char *name, int kind);
extern struct symbol *define_symbol(struct symbol_table *table, const char *name, int kind);
extern int symbol_scope_begin(struct symbol_table *table);
extern int symbol_scope_end(struct symbol_table *table);

extern int get_symbol_count(const struct symbol_table *table);
extern struct symbol *get_symbol(struct symbol_table *table, int index);
extern void print_symbol_table(const struct symbol_table *table);

#endif /* _H */
