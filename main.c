#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gen_x86.h"
#include "lexer.h"
#include "message.h"
#include "parse.h"
#include "semantics.h"
#include "string_table.h"
#include "preprocessor.h"

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

struct option {
    int print_tree;
    int print_preprocess;
};

static int compile(const char *filename, const struct option *opt);

int main(int argc, char **argv)
{
    struct option opt = {0};
    const char *infile = NULL;
    int ret = 0;

    if (argc == 1) {
        printf("acc: error: no input files\n");
        return -1;
    }

    if (argc == 3) {
        if (!strcmp(argv[1], "--print-tree")) {
            opt.print_tree = 1;
            infile = argv[2];
        }
        else if (!strcmp(argv[1], "--print-preprocess")) {
            opt.print_preprocess = 1;
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

    ret = compile(infile, &opt);

    return ret;
}

static int compile(const char *filename, const struct option *opt)
{
    struct preprocessor *pp = NULL;
    struct string_table *strtab = NULL;
    struct symbol_table *symtab = NULL;
    struct parser *parser = NULL;
    struct ast_node *tree = NULL;
    struct message_list *messages = NULL;
    FILE *fp = NULL;
    char output[256] = {'\0'};

    fp = fopen(filename, "r");
    if (!fp) {
        printf("acc: error: no input files\n");
        return -1;
    }

    pp = new_preprocessor();
    strtab = new_string_table();
    symtab = new_symbol_table();
    messages = new_message_list();
    messages->strtab = strtab;

    parser = new_parser();
    parser->lex.file = fp;
    parser->lex.strtab = strtab;
    parser->symtab = symtab;
    parser->msg = messages;

    /* ------------------------- */
    preprocess_text(pp, filename);

    if (opt->print_preprocess) {
        printf("%s", pp->text->buf);
        goto finalize;
    }

    /* ------------------------- */
    tree = parse(parser);
    semantic_analysis(tree, symtab, messages);

    if (opt->print_tree) {
        print_tree(tree);
        printf("\n");
        print_symbol_table(symtab);
        goto finalize;
    }

    /* ------------------------- */
    if (messages->warning_count > 0)
        print_warning_messages(fp, filename, messages);

    if (messages->error_count > 0) {
        print_error_messages(fp, filename, messages);

        fclose(fp);
        exit(EXIT_FAILURE);
    }

    fclose(fp);

    /* ------------------------- */
    make_output_filename(filename, output);
    fp = fopen(output, "w");
    if (!fp) {
        return -1;
    }
    gen_x86(fp, tree, symtab);

    fclose(fp);

finalize:
    /* ------------------------- */
    free_message_list(messages);
    free_parser(parser);
    free_symbol_table(symtab);
    free_string_table(strtab);
    free_preprocessor(pp);

    return 0;
}
