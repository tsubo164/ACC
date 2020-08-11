#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include "lexer.h"

static int get_max_offset(const struct ast_node *node)
{
    int max, l, r;

    if (node == NULL) {
        return 0;
    }

    l = get_max_offset(node->l);
    r = get_max_offset(node->r);
    max = l > r ? l : r;

    if (node->kind == NOD_VAR) {
        return node->value > max ? node->value : max;
    } else {
        return max;
    }

}

static void gen_code(FILE *file, const struct ast_node *node)
{
    static int label_id = 0;

    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_LIST:
        gen_code(file, node->l);
        gen_code(file, node->r);
        break;

    case NOD_IF:
        /* if */
        gen_code(file, node->l);
        fprintf(file, "  cmp rax, 0\n");
        fprintf(file, "  je .L%03d_0\n", label_id);
        /* then */
        gen_code(file, node->r->l);
        fprintf(file, "  jmp .L%03d_1\n", label_id);
        /* else */
        fprintf(file, ".L%03d_0:\n", label_id);
        gen_code(file, node->r->r);
        fprintf(file, ".L%03d_1:\n", label_id);
        label_id++;
        break;

    case NOD_WHILE:
        fprintf(file, ".L%03d_0:\n", label_id);
        gen_code(file, node->l);
        fprintf(file, "  cmp rax, 0\n");
        fprintf(file, "  je .L%03d_1\n", label_id);
        gen_code(file, node->r);
        fprintf(file, "  jmp .L%03d_0\n", label_id);
        fprintf(file, ".L%03d_1:\n", label_id);
        label_id++;
        break;

    case NOD_RETURN:
        gen_code(file, node->l);
        fprintf(file, "  mov rsp, rbp\n");
        fprintf(file, "  pop rbp\n");
        fprintf(file, "  ret\n");
        break;

    case NOD_VAR:
        fprintf(file, "  mov rax, rbp\n");
        fprintf(file, "  sub rax, %d\n", node->value);

        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  mov rax, [rdx]\n");
        break;

    case NOD_ASSIGN:
        /* assuming node->l is var */
        fprintf(file, "  mov rax, rbp\n");
        fprintf(file, "  sub rax, %d\n", node->l->value);
        /*
        gen_code(file, node->l);
        */
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  pop rdx\n");
        fprintf(file, "  mov [rdx], rax\n");

        fprintf(file, "  mov rax, [rdx]\n");
        break;

    case NOD_NUM:
        fprintf(file, "  mov rax, %d\n", node->value);
        break;

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

    case NOD_LT:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setl al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_GT:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setg al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_LE:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setle al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_GE:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setge al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_EQ:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  pop rdx\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  sete al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_NE:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  pop rdx\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setne al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    default:
        break;
    }
}

#define TERMINAL_COLOR_BLACK   "\x1b[30m"
#define TERMINAL_COLOR_RED     "\x1b[31m"
#define TERMINAL_COLOR_GREEN   "\x1b[32m"
#define TERMINAL_COLOR_YELLOW  "\x1b[33m"
#define TERMINAL_COLOR_BLUE    "\x1b[34m"
#define TERMINAL_COLOR_MAGENTA "\x1b[35m"
#define TERMINAL_COLOR_CYAN    "\x1b[36m"
#define TERMINAL_COLOR_WHITE   "\x1b[37m"
#define TERMINAL_COLOR_RESET   "\x1b[39m"

#define TERMINAL_DECORATION_BOLD    "\x1b[1m"
#define TERMINAL_DECORATION_RESET   "\x1b[0m"

void print_error_message(const struct parser *p, const char *filename)
{
    FILE *fp = p->lex.file;
    long err_pos = p->error_pos;
    int found_error_location = 0;
    int err_col, err_row;
    int x = 0, y = 1; /* scanning pos row and column */
    int c = '\0';
    char line[1024] = {'\0'};

    fseek(fp, 0L, SEEK_SET);

    for (;;) {
        if (ftell(fp) == err_pos) {
            found_error_location = 1;
            err_col = x;
            err_row = y;
        }

        c = fgetc(fp);

        if (c == '\n' || c == EOF) {
            if (found_error_location) {
                line[x] = '\0';
                break;
            } else {
                y++;
                x = 0;
                line[x] = '\0';
            }
        } else {
            line[x++] = c;
        }
    }

    printf(TERMINAL_DECORATION_BOLD);
        printf("%s:", filename);
        printf("%d:%d: ", err_row, err_col);
        printf(TERMINAL_COLOR_RED);
            printf("error: ");
        printf(TERMINAL_COLOR_RESET);
        printf("%s\n", p->error_msg);
    printf(TERMINAL_DECORATION_RESET);

    printf("%s\n", line);
    printf("%*s", err_col, "");

    printf(TERMINAL_DECORATION_BOLD);
    printf(TERMINAL_COLOR_GREEN);
        printf("%c\n", '^');
    printf(TERMINAL_COLOR_RESET);
    printf(TERMINAL_DECORATION_RESET);
}

int main(int argc, char **argv)
{
    struct ast_node *node;
    struct parser parser;
    FILE *file = NULL;

    if (argc != 2) {
        printf("mcc: error: no input files\n");
        return -1;
    }

    file = fopen(argv[1], "r");
    if (!file) {
        printf("mcc: error: no input files\n");
        return -1;
    }

    parser_init(&parser);
    parser.lex.file = file;

    {
        node = parse(&parser);

        if (parser.error_pos >= 0) {
            print_error_message(&parser, argv[1]);

            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);

    file = fopen("input.s", "w");
    if (!file) {
        return -1;
    }

    fprintf(file, ".intel_syntax noprefix\n");
    fprintf(file, ".global _main\n");
    fprintf(file, "_main:\n");

    fprintf(file, "  push rbp\n");
    fprintf(file, "  mov rbp, rsp\n");
    fprintf(file, "  sub rsp, %d\n", get_max_offset(node));

    gen_code(file, node);

    fprintf(file, "  mov rsp, rbp\n");
    fprintf(file, "  pop rbp\n");

    fprintf(file, "  ret\n");
    fclose(file);

    return 0;
}
