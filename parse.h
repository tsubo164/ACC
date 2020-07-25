#ifndef PARSE_H
#define PARSE_H

#include "lexer.h"

#define TOKEN_BUFFER_SIZE 2

struct parser {
  struct lexer lex;
  struct token tokbuf[TOKEN_BUFFER_SIZE];
  int head, curr;
};

enum ast_node_kind {
  NOD_NUM,
  NOD_ADD,
  NOD_SUB,
  NOD_MUL
};

struct ast_node {
  enum ast_node_kind kind;
  struct ast_node *l;
  struct ast_node *r;
  int value;
};

extern void parser_init(struct parser *p);

extern struct ast_node *additive_expression(struct parser *p);

#endif /* _H */
