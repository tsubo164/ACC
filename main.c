#include <stdio.h>
#include "parse.h"
#include "lexer.h"

static void gen_code(FILE *file, struct ast_node *node)
{
  switch (node->kind) {
  case NOD_ADD:
    gen_code(file, node->l);
    fprintf(file, "  push rax\n");
    gen_code(file, node->r);
    fprintf(file, "  pop rdx\n");
    fprintf(file, "  add rax, rdx\n");
    break;

  case NOD_SUB:
    gen_code(file, node->l);
    fprintf(file, "  push rax\n");
    gen_code(file, node->r);
    fprintf(file, "  mov rdx, rax\n");
    fprintf(file, "  pop rax\n");
    fprintf(file, "  sub rax, rdx\n");
    break;

  case NOD_MUL:
    gen_code(file, node->l);
    fprintf(file, "  push rax\n");
    gen_code(file, node->r);
    fprintf(file, "  pop rdx\n");
    fprintf(file, "  imul rax, rdx\n");
    break;

  case NOD_DIV:
    gen_code(file, node->l);
    fprintf(file, "  push rax\n");
    gen_code(file, node->r);
    fprintf(file, "  mov rdi, rax\n");
    fprintf(file, "  pop rax\n");
    fprintf(file, "  cqo\n");
    fprintf(file, "  idiv rdi\n");
    break;

  case NOD_NUM:
    fprintf(file, "  mov rax, %d\n", node->value);
    break;

  default:
    break;
  }
}

int main(int argc, char **argv)
{
  struct ast_node *node;
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

  parser_init(&parser);
  parser.lex.file = file;

  {
    node = additive_expression(&parser);
    n = node->l->value;
    m = node->r->value;
  }

  fclose(file);

  file = fopen("input.s", "w");
  if (!file) {
    return -1;
  }

  fprintf(file, ".intel_syntax noprefix\n");
  fprintf(file, ".global _main\n");
  fprintf(file, "_main:\n");

  gen_code(file, node);

  fprintf(file, "  ret\n");
  fclose(file);

  return 0;
}
