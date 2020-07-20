#ifndef PARSE_H
#define PARSE_H

#include "lexer.h"

struct parser {
  struct lexer lex;
  struct token tok;
};

extern int additive_expression(struct parser *p, int *l, int *r);

#endif /* _H */
