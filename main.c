#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gen_x86.h"
#include "lexer.h"
#include "message.h"
#include "parse.h"
#include "semantics.h"
#include "string_table.h"

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
    struct string_table *strtab = NULL;
    struct parser *parser = NULL;
    struct ast_node *tree = NULL;
    struct message_list *messages = NULL;

    FILE *file = NULL;
    char output[256] = {'\0'};

    const char *infile = NULL;
    int do_print_tree = 0;

    if (argc == 1) {
        printf("acc: error: no input files\n");
        return -1;
    }

    if (argc == 3) {
        if (!strcmp(argv[1], "--print-tree")) {
            do_print_tree = 1;
            infile = argv[2];
        } else {
            printf("acc: error: unsupported option '%s'\n", argv[1]);
            return -1;
        }
    }
    else if (argc == 2) {
        infile = argv[1];
    }
    else {
        printf("acc: error: too many arguments\n");
    }

    file = fopen(infile, "r");
    if (!file) {
        printf("acc: error: no input files\n");
        return -1;
    }

    strtab = new_string_table();
    parser = new_parser();
    parser->lex.file = file;
    parser->lex.strtab = strtab;

    messages = new_message_list();

    tree = parse(parser);

    if (do_print_tree) {
        print_tree(tree);
    }

    /*
    symbol_assign_local_storage(&parser->symtbl);
    */
    semantic_analysis(tree, &parser->symtbl, messages);

    /* ------------------------- */
    if (messages->error_count > 0) {

        print_error_messages(file, argv[1], messages);

        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    /* ------------------------- */
    make_output_filename(infile, output);
    file = fopen(output, "w");
    if (!file) {
        return -1;
    }
    gen_x86(file, tree, &parser->symtbl);

    fclose(file);

    /* ------------------------- */
    free_message_list(messages);
    free_parser(parser);
    free_string_table(strtab);

    return 0;
}
