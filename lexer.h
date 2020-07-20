#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

enum token_kind {
  TK_UNKNOWN = -1,
  TK_PLUS,
  TK_NUM,
  TK_EOF
};

struct token {
  int kind;
  int value;
};

struct lexer {
  FILE *file;
};

extern enum token_kind lex_get_token(struct lexer *l, struct token *tok);

#endif /* _H */
