#include <stdio.h>
#include <ctype.h>

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

static int readc(struct lexer *l)
{
  const int c = fgetc(l->file);
  return c;
}

enum token_kind lex_get_token(struct lexer *l, struct token *tok)
{
  int c = '\n';

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

int main(int argc, char **argv)
{
  struct lexer lex;
  struct token tok;
  FILE *file = NULL;
  int n = 0, m = 0;
  enum token_kind kind = TK_UNKNOWN;

  if (argc != 2) {
    printf("mcc: error: no input files\n");
    return -1;
  }

  file = fopen(argv[1], "r");
  if (!file) {
    return -1;
  }

  lex.file = file;

  kind = lex_get_token(&lex, &tok);
  if (kind == TK_NUM) {
    n = tok.value;
  } else {
    printf("error:\n");
    fclose(file);
    return -1;
  }
  kind = lex_get_token(&lex, &tok);
  if (kind == TK_PLUS) {
  } else {
    printf("error:\n");
    fclose(file);
    return -1;
  }
  kind = lex_get_token(&lex, &tok);
  if (kind == TK_NUM) {
    m = tok.value;
  } else {
    printf("error:\n");
    fclose(file);
    return -1;
  }

  fclose(file);

  file = fopen("input.s", "w");
  if (!file) {
    return -1;
  }

  fprintf(file, ".intel_syntax noprefix\n");
  fprintf(file, ".global _main\n");
  fprintf(file, "_main:\n");
  fprintf(file, "  mov rax, %d\n", n);
  fprintf(file, "  add rax, %d\n", m);
  fprintf(file, "  ret\n");
  fclose(file);

  return 0;
}
