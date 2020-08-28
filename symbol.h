#ifndef SYMBOL_H
#define SYMBOL_H

enum symbol_kind {
    SYM_SCOPE_BEGIN,
    SYM_SCOPE_END,
    SYM_VAR,
    SYM_FUNC,
    SYM_PARAM
};

struct symbol {
    char *name;
    int kind;
    int offset;

    int scope_level;
};

#define SYMBOL_TABLE_SIZE 128
struct symbol_table {
    struct symbol data[SYMBOL_TABLE_SIZE];
    int symbol_count;
    int nvars;

    int current_scope_level;
};

extern void init_symbol_table(struct symbol_table *table);

extern const struct symbol *lookup_symbol(const struct symbol_table *table,
        const char *name, enum symbol_kind kind);

extern const struct symbol *insert_symbol(struct symbol_table *table,
        const char *name, enum symbol_kind kind);

extern int symbol_scope_begin(struct symbol_table *table);
extern int symbol_scope_end(struct symbol_table *table);

#endif /* _H */
