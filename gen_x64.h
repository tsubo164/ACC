#ifndef GEN_X86_H
#define GEN_X86_H

#include "ast.h"

extern void gen_x64(FILE *fp,
        const struct ast_node *tree, const struct symbol_table *table);

#endif /* _H */
