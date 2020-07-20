#include <stdio.h>
#include "parse.h"
#include "lexer.h"

int main(int argc, char **argv)
{
  struct parser parser;
  FILE *file = NULL;
  int n = 0, m = 0;

  if (argc != 2) {
    printf("mcc: error: no input files\n");
    return -1;
  }

  file = fopen(argv[1], "r");
  if (!file) {
    return -1;
  }

  parser.lex.file = file;

  additive_expression(&parser, &n, &m);

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
