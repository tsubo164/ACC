#ifndef SYMBOL_H
#define SYMBOL_H

enum symbol_kind {
    SYM_VAR,
    SYM_FUNC,
    SYM_PARAM
};

struct symbol {
    char *name;
    int kind;
    int offset;
};

#define SYMBOL_TABLE_SIZE 128
struct symbol_table {
    struct symbol table[SYMBOL_TABLE_SIZE];
    int nsyms;
    int nvars;
};

extern void init_symbol_table(struct symbol_table *symtbl);

extern const struct symbol *lookup_symbol(const struct symbol_table *symtbl,
        const char *name, enum symbol_kind kind);

extern const struct symbol *insert_symbol(struct symbol_table *symtbl,
        const char *name, enum symbol_kind kind);

#endif /* _H */
