#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "ast.h"
#include "diagnostic.h"

extern int semantic_analysis(struct ast_node *tree,
        struct symbol_table *table, struct diagnostic *diag);

#endif /* _H */
