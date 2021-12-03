#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gen_x64.h"
#include "ast.h"
#include "lexer.h"
#include "message.h"
#include "parse.h"
#include "semantics.h"
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
    struct symbol_table *symtab = NULL;
    struct message_list *messages = NULL;
    struct parser *parser = NULL;
    struct ast_node *tree = NULL;
    FILE *fp = NULL;
    char output[256] = {'\0'};
    int ret = 0;

    /* preprocess */
    pp = new_preprocessor();
    preprocess_text(pp, filename);

    if (opt->print_preprocess) {
        printf("%s", pp->text->buf);
        goto finalize;
    }

    /* parse */
    symtab = new_symbol_table();
    messages = new_message_list();

    parser = new_parser();
    parser->symtab = symtab;
    parser->msg = messages;

    parser->lex->head = pp->text->buf;
    parser->lex->next = pp->text->buf;

    tree = parse(parser);

    /* analyze semantics */
    semantic_analysis(tree, symtab, messages);

    if (opt->print_tree) {
        print_tree(tree);
        printf("\n");
        print_symbol_table(symtab);
        goto finalize;
    }

    /* diagnostics */
    if (messages->warning_count > 0)
        print_warning_messages(messages);

    if (messages->error_count > 0) {
        print_error_messages(messages);
        ret = EXIT_FAILURE;
        goto finalize;
    }

    /* generate code */
    make_output_filename(filename, output);
    fp = fopen(output, "w");
    if (!fp) {
        ret = EXIT_FAILURE;
        goto finalize;
    }
    gen_x64(fp, tree, symtab);
    fclose(fp);

finalize:
    free_ast_node(tree);
    free_parser(parser);
    free_message_list(messages);
    free_symbol_table(symtab);
    free_preprocessor(pp);

    return ret;
}
