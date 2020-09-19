#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "ast.h"
#include "message.h"

extern int semantic_analysis(struct ast_node *tree,
        struct symbol_table *table, struct message_list *messages);

#endif /* SEMANTICS_H */
