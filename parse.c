#include <stdlib.h>
#include "parse.h"

struct ast_node *new_node(enum ast_node_kind kind)
{
  struct ast_node *n = malloc(sizeof(struct ast_node));
  n->kind = kind;

  return n;
}

static enum token_kind gettok(struct parser *p)
{
  return lex_get_token(&p->lex, &p->tok);
}

/* XXX typedef TEST */
typedef struct token token;
typedef struct ast_node node;
/*
*/

/*
 * returns pointer to the token if that is one caller queries otherwise NULL
 */
static const struct token *consume(struct parser *p, enum token_kind query)
{
  enum token_kind kind = gettok(p);
  if (kind == query) {
    return &p->tok;
  } else {
    return NULL;
  }
  return (kind == query) ? &p->tok : NULL;
}

/*
 * unary_expression
 *     : TK_NUM
 *     ;
 */
struct ast_node *unary_expression(struct parser *p)
{
  const token *tok = consume(p, TK_NUM);
  node *nod = NULL;

  if (tok) {
    nod = new_node(ND_NUM);
    nod->value = tok->value;
    return nod;
  }
  else {
    printf("error: not a number\n");
    return NULL;
  }
}

/*
 * additive_expression
 *     : unary_expression
 *     | additive_expression '+' unary_expression
 *     ;
 */
struct ast_node *additive_expression(struct parser *p)
{
  struct ast_node *n1, *n2, *n3;
  enum token_kind kind = TK_UNKNOWN;

  n1 = unary_expression(p);

  kind = gettok(p);
  if (kind == TK_PLUS) {
    n2 = new_node(ND_ADD);
  } else {
    return NULL;
  }

  n3 = unary_expression(p);

  n2->l = n1;
  n2->r = n3;

  return n2;
}
