#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "token.h"

struct lexer {
  FILE *file;
};

extern enum token_kind lex_get_token(struct lexer *l, struct token *tok);

#endif /* _H */
