#include <stdio.h>

int main()
{
  FILE *file;

  file = fopen("a.s", "w");
  if (!file) {
    return -1;
  }

  fprintf(file, ".intel_syntax noprefix\n");
  fprintf(file, ".global _main\n");
  fprintf(file, "_main:\n");
  fprintf(file, "  mov rax, 42\n");
  fprintf(file, "  ret\n");
  fclose(file);

  return 0;
}
