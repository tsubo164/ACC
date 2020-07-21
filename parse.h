#ifndef PARSE_H
#define PARSE_H

#include "lexer.h"

struct parser {
  struct lexer lex;
  struct token tok;
};

enum ast_node_kind {
  ND_NUM,
  ND_ADD
};

struct ast_node {
  enum ast_node_kind kind;
  struct ast_node *l;
  struct ast_node *r;
  int value;
};

extern struct ast_node *additive_expression(struct parser *p);

#endif /* _H */
