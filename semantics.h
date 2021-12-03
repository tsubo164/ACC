#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "ast.h"
#include "diagnostic.h"

extern int analyze_semantics(struct ast_node *tree,
        struct symbol_table *table, struct diagnostic *diag);

#endif /* _H */
