#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "ast.h"

extern int semantic_analysis(struct ast_node *tree, struct symbol_table *table);

#endif /* SEMANTICS_H */
