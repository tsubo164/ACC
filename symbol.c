#include <stdlib.h>
#include <string.h>
#include "symbol.h"

static void init_symbol(struct symbol *sym)
{
    sym->name = NULL;
    sym->kind = SYM_VAR;
    sym->offset = 0;
}

void init_symbol_table(struct symbol_table *symtbl)
{
    int i;

    for (i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        init_symbol(&symtbl->table[i]);
    }

    symtbl->nsyms = 0;
    symtbl->nvars = 0;
}

const struct symbol *lookup_symbol(const struct symbol_table *symtbl,
        const char *name, enum symbol_kind kind)
{
    int i;

    for (i = 0; i < symtbl->nsyms; i++) {
        const struct symbol *sym = &symtbl->table[i];

        if (!strcmp(sym->name, name) /*&& sym->kind == kind*/) {
            return sym;
        }
    }

    return NULL;
}

const struct symbol *insert_symbol(struct symbol_table *symtbl,
        const char *name, enum symbol_kind kind)
{
    struct symbol *sym = NULL;

    if (lookup_symbol(symtbl, name, kind) != NULL) {
        return NULL;
    }

    if (kind == SYM_FUNC) {
        symtbl->nvars = 0;
    }

    sym = &symtbl->table[symtbl->nsyms++];

    sym->name = malloc(strlen(name) + 1);
    strcpy(sym->name, name);
    sym->kind = kind;
    sym->offset = 0;

    if (kind == SYM_VAR || kind == SYM_PARAM) {
        symtbl->nvars++;
        sym->offset = 8 * symtbl->nvars;
    }

    return sym;
}
