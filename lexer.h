#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

enum token_kind {
  TK_UNKNOWN = -1,
  TK_NUM = 256,
  /*
  TK_PLUS,
  TK_MINUS,
  TK_STAR,
  TK_SLASH,
  */
  TK_EOF
};

struct token {
  /*
  enum token_kind kind;
  */
  int kind;
  int value;
};

struct lexer {
  FILE *file;
};

extern void token_init(struct token *tok);
extern enum token_kind kind_of(const struct token *tok);

extern void lexer_init(struct lexer *lex);

extern enum token_kind lex_get_token(struct lexer *l, struct token *tok);

#endif /* _H */
