#include <ctype.h>
#include "lexer.h"

static int readc(struct lexer *l)
{
  const int c = fgetc(l->file);
  return c;
}

enum token_kind lex_get_token(struct lexer *l, struct token *tok)
{
  int c = '\0';

state_initial:
  c = readc(l);

  switch (c) {

  /* number */
  case '.':
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    tok->kind = TK_NUM;
    tok->value = c - '0';
    goto state_final;

  /* whitespace */
  case '+':
    tok->kind = TK_PLUS;
    goto state_final;

  /* whitespace */
  case ' ':
    goto state_initial;

  /* eof */
  case EOF:
    tok->kind = TK_EOF;
    goto state_final;

  /* unknown */
  default:
    tok->kind = TK_UNKNOWN;
    goto state_final;
  }

state_final:
  return tok->kind;
}
