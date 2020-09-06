#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "lexer.h"
#include "gen_x86.h"

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

void make_output_filename(const char *input, char *output)
{
    const size_t len = strlen(input);
    if (len > 255) {
        return;
    }

    strcpy(output, input);
    if (output[len - 1] == 'c') {
        output[len - 1] = 's';
    }
}

int main(int argc, char **argv)
{
    struct ast_node *tree;
    struct parser parser;
    FILE *file = NULL;
    char output[256] = {'\0'};

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
        tree = parse(&parser);

        if (parser.error_pos >= 0) {
            print_error_message(&parser, argv[1]);

            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);

    make_output_filename(argv[1], output);
    file = fopen(output, "w");
    if (!file) {
        return -1;
    }

    symbol_assign_local_storage(&parser.symtbl);
    gen_x86(file, tree);

    fclose(file);

    return 0;
}
