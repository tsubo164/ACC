#include "parse.h"

int additive_expression(struct parser *p, int *l, int *r)
{
  enum token_kind kind = TK_UNKNOWN;

  kind = lex_get_token(&p->lex, &p->tok);
  if (kind == TK_NUM) {
    *l = p->tok.value;
  } else {
    printf("error:\n");
    return -1;
  }
  kind = lex_get_token(&p->lex, &p->tok);
  if (kind == TK_PLUS) {
  } else {
    printf("error:\n");
    return -1;
  }
  kind = lex_get_token(&p->lex, &p->tok);
  if (kind == TK_NUM) {
    *r = p->tok.value;
  } else {
    printf("error:\n");
    return -1;
  }
  return 0;
}
