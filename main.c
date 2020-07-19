#include <stdio.h>
#include <ctype.h>

int main(int argc, char **argv)
{
  FILE *file = NULL;
  int c = '\0';
  int n = 0;

  if (argc != 2) {
    printf("mcc: error: no input files\n");
    return -1;
  }

  file = fopen(argv[1], "r");
  if (!file) {
    return -1;
  }

  c = fgetc(file);
  if (!isdigit(c)) {
    printf("error: the character is not a digit: %c\n", c);
    fclose(file);
    return -1;
  }
  fclose(file);

  file = fopen("input.s", "w");
  if (!file) {
    return -1;
  }

  n = c - '0';

  fprintf(file, ".intel_syntax noprefix\n");
  fprintf(file, ".global _main\n");
  fprintf(file, "_main:\n");
  fprintf(file, "  mov rax, %d\n", n);
  fprintf(file, "  ret\n");
  fclose(file);

  return 0;
}
