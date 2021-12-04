#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gen_x64.h"
#include "ast.h"
#include "lexer.h"
#include "diagnostic.h"
#include "parse.h"
#include "semantics.h"
#include "preprocessor.h"

struct option {
    const char *out_filename;
    int preprocess;
    int preprocess_compile;
    int preprocess_compile_assemble;
    int print_tree;
};

static int is_filename_x(const char *name, int ext)
{
    const char *s;
    int len = 0;

    for(s = name; *s; s++, len++)
        if (!isalnum(*s) && !strchr("/.-_", *s))
            return 0;

    if (len < 3)
        return 0;

    if (name[len-2] == '.' && name[len-1] == ext)
        return 1;

    return 0;
}

static int compile(const char *infile, const struct option *opt);

int main(int argc, char **argv)
{
    struct option opt = {0};
    const char *infile = NULL;
    char **argp = argv + 1;
    char **endp = argv + argc;

    while (argp != endp) {
        if (!strcmp("-E", *argp)) {
            opt.preprocess = 1;
        }
        else if (!strcmp("-S", *argp)) {
            opt.preprocess_compile = 1;
        }
        else if (!strcmp("-c", *argp)) {
            opt.preprocess_compile_assemble = 1;
        }
        else if (!strcmp("-o", *argp)) {
            if (++argp == endp) {
                printf("acc: error: missing file name after '-o'\n");
                return 1;
            }
            if (is_filename_x(*argp, 's'))
                opt.out_filename = *argp;
        }
        else if (is_filename_x(*argp, 'c')) {
            infile = *argp;
        }
        else if (!strcmp("--print-tree", *argp)) {
            opt.print_tree = 1;
        }
        else if (strlen(*argp) > 0 && *argp[0] == '-') {
            printf("acc: error: unsupported option '%s'\n", *argp);
            return 1;
        }
        else {
        }

        argp++;
    }

    if (!infile) {
        printf("acc: error: no input files\n");
        return 1;
    }

    if (!opt.preprocess_compile_assemble &&
        !opt.preprocess_compile &&
        !opt.preprocess &&
        !opt.print_tree) {
        printf("acc: error: use -E, -S, -c or --print-tree option\n");
        return 1;
    }

    return compile(infile, &opt);
}

static void make_output_filename(const char *input, char *output, size_t size)
{
    const size_t len = strlen(input);
    if (len > size - 1)
        return;

    strcpy(output, input);
    if (output[len - 1] == 'c')
        output[len - 1] = 's';
}

static int compile(const char *infile, const struct option *opt)
{
    struct preprocessor *pp = NULL;
    struct symbol_table *symtab = NULL;
    struct diagnostic *diag = NULL;
    struct parser *parser = NULL;
    struct ast_node *tree = NULL;
    FILE *fp = NULL;
    char outfile[256] = {'\0'};
    int ret = 0;

    /* preprocess */
    pp = new_preprocessor();
    preprocess_file(pp, infile);

    if (opt->preprocess) {
        printf("%s", get_text(pp));
        goto finalize;
    }

    /* parse */
    symtab = new_symbol_table();
    diag = new_diagnostic();

    parser = new_parser();
    tree = parse_text(parser, get_text(pp), symtab, diag);

    /* semantics */
    analyze_semantics(tree, symtab, diag);

    if (opt->print_tree) {
        print_tree(tree);
        printf("\n");
        print_symbol_table(symtab);
        goto finalize;
    }

    /* diagnostics */
    if (diag->warning_count > 0)
        print_warnings(diag);

    if (diag->error_count > 0) {
        print_errors(diag);
        ret = 1;
        goto finalize;
    }

    /* compile */
    if (opt->preprocess_compile) {
        if (opt->out_filename)
            strcpy(outfile, opt->out_filename);
        else
            make_output_filename(infile, outfile, sizeof(outfile)/sizeof(outfile[0]));

        fp = fopen(outfile, "w");
        if (!fp) {
            ret = 1;
            goto finalize;
        }
        gen_x64(fp, tree, symtab);
        fclose(fp);
    }

finalize:
    free_ast_node(tree);
    free_parser(parser);
    free_diagnostic(diag);
    free_symbol_table(symtab);
    free_preprocessor(pp);

    return ret;
}
