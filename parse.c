#include <stdlib.h>
#include "parse.h"

static struct ast_node *new_node(enum ast_node_kind kind,
    struct ast_node *l, struct ast_node *r)
{
  struct ast_node *n = malloc(sizeof(struct ast_node));
  n->kind = kind;
  n->l = l;
  n->r = r;
  n->value = 0;

  return n;
}

static enum token_kind gettok(struct parser *p)
{
  enum token_kind kind;
  const int N = TOKEN_BUFFER_SIZE;

  if (p->head == p->curr) {
    p->head = (p->head + 1) % N;
    p->curr = (p->curr + 1) % N;
  } else {
    p->curr = (p->curr + 1) % N;
    return p->tokbuf[p->head].kind;
  }

  kind = lex_get_token(&p->lex, &p->tokbuf[p->head]);
  return kind;
}

/* XXX typedef TEST */
typedef struct token token;
typedef struct ast_node node;

/*
 * returns pointer to the token if that is one caller queries otherwise NULL
 */
static const struct token *consume(struct parser *p, enum token_kind query)
{
  const enum token_kind kind = gettok(p);
  const int N = TOKEN_BUFFER_SIZE;
  if (kind == query) {
    return &p->tokbuf[p->head];
  } else {
    p->curr = (p->curr - 1 + N) % N;
    return NULL;
  }
}

void parser_init(struct parser *p)
{
  int i;

  for (i = 0; i < TOKEN_BUFFER_SIZE; i++) {
    token_init(&p->tokbuf[i]);
  }
  lexer_init(&p->lex);
  p->head = 0;
  p->curr = 0;
}

/*
 * multiplicative_expression
 *     : TK_NUM
 *     ;
 */
struct ast_node *multiplicative_expression(struct parser *p)
{
  const token *tok = consume(p, TK_NUM);
  node *nod = NULL;

  if (tok) {
    nod = new_node(NOD_NUM, NULL, NULL);
    nod->value = tok->value;
    return nod;
  } else {
    printf("error: not a number\n");
    return NULL;
  }
}

/*
 * additive_expression
 *     : multiplicative_expression
 *     | additive_expression '+' multiplicative_expression
 *     ;
 */
struct ast_node *additive_expression(struct parser *p)
{
  node *base = multiplicative_expression(p);
  for (;;) {
    if (consume(p, '+')) {
      base = new_node(NOD_ADD, base, multiplicative_expression(p));
    } else {
      return base;
    }
  }
}
