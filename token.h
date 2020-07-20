#ifndef TOKEN_H
#define TOKEN_H

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

#endif /* _H */
