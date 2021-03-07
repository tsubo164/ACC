#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"
#include "position.h"

enum symbol_kind {
    SYM_SCOPE_BEGIN,
    SYM_SCOPE_END,
    SYM_SWITCH_BEGIN,
    SYM_SWITCH_END,
    SYM_CASE,
    SYM_DEFAULT,
    SYM_LABEL,
    SYM_STRING,
    SYM_VAR,
    SYM_FUNC,
    SYM_PARAM,
    SYM_MEMBER,
    SYM_ENUMERATOR,
    SYM_TAG_STRUCT,
    SYM_TAG_ENUM,
    SYM_TYPEDEF
};

struct symbol {
    int kind;
    const char *name;
    struct data_type *type;
    int scope_level;
    int mem_offset;
    int id;
    struct position pos;

    struct symbol *next;
    struct symbol *prev;

    /* flags */
    char is_extern;
    char is_static;
    char is_defined;
    char is_redefined;
    char is_initialized;
    char is_assigned;
    char is_used;
};

struct symbol_table {
    struct symbol *head;
    struct symbol *tail;

    int current_scope_level;
    int current_switch_level;
};

/* symbol */
extern int is_extern(const struct symbol *sym);
extern int is_static(const struct symbol *sym);
extern int is_global_var(const struct symbol *sym);
extern int is_local_var(const struct symbol *sym);
extern int is_func(const struct symbol *sym);
extern int is_param(const struct symbol *sym);
extern int is_struct_tag(const struct symbol *sym);
extern int is_member(const struct symbol *sym);
extern int is_enum_tag(const struct symbol *sym);
extern int is_enumerator(const struct symbol *sym);
extern int is_case(const struct symbol *sym);
extern int is_default(const struct symbol *sym);
extern int is_label(const struct symbol *sym);
extern int is_typedef(const struct symbol *sym);
extern int is_string_literal(const struct symbol *sym);
extern const char *symbol_to_string(const struct symbol *sym);

/* symbol table */
extern struct symbol_table *new_symbol_table();
extern void free_symbol_table(struct symbol_table *table);
extern void print_symbol_table(const struct symbol_table *table);

/* general symobl */
extern struct symbol *define_symbol(struct symbol_table *table,
        const char *name, int kind, struct data_type *type);
extern struct symbol *use_symbol(struct symbol_table *table, const char *name, int kind);

/* struct member symobl */
struct symbol *use_struct_member_symbol(struct symbol_table *table,
        struct symbol *strct, const char *member);

/* case symobl */
extern struct symbol *define_case_symbol(struct symbol_table *table, int kind, int case_value);

/* label symobl */
extern struct symbol *define_label_symbol(struct symbol_table *table, const char *label);
extern struct symbol *use_label_symbol(struct symbol_table *table, const char *label);

/* string symobl */
extern struct symbol *define_string_symbol(struct symbol_table *table, const char *str);

/* type name symobl */
extern struct symbol *find_type_name_symbol(struct symbol_table *table, const char *name);

/* scope symobl */
extern int symbol_scope_begin(struct symbol_table *table);
extern int symbol_scope_end(struct symbol_table *table);
extern int symbol_switch_begin(struct symbol_table *table);
extern int symbol_switch_end(struct symbol_table *table);

/* sizes */
extern void compute_struct_size(struct symbol *strc);
extern void compute_enum_size(struct symbol *enm);

#endif /* _H */
