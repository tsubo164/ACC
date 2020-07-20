#include <stdio.h>
#include "lexer.h"

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
