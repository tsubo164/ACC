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
    int preprocess;
    int preprocess_compile;
    int preprocess_compile_assemble;
    int print_tree;
};

static int is_c_filename(const char *name)
{
    const char *s;
    int len;

    for(s = name; *s; s++)
        if (!isalnum(*s) && !strchr("/.-_", *s))
            return 0;

    len = s - name;
    if (len < 3)
        return 0;

    if (*(s-2) == '.' && *(s-1) == 'c')
        return 1;
    return 0;
}

static int compile(const char *filename, const struct option *opt);

int main(int argc, char **argv)
{
    struct option opt = {0};
    const char *infile = NULL;
    char **argp = argv + 1;
    char **endp = argv + argc;
    int ret = 0;

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
        else if (!strcmp("--print-tree", *argp)) {
            opt.print_tree = 1;
        }
        else if (is_c_filename(*argp)) {
            infile = *argp;
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

    ret = compile(infile, &opt);

    return ret;
}

static void make_output_filename(const char *input, char *output)
{
    const size_t len = strlen(input);
    if (len > 255)
        return;

    strcpy(output, input);
    if (output[len - 1] == 'c')
        output[len - 1] = 's';
}

static int compile(const char *filename, const struct option *opt)
{
    struct preprocessor *pp = NULL;
    struct symbol_table *symtab = NULL;
    struct diagnostic *diag = NULL;
    struct parser *parser = NULL;
    struct ast_node *tree = NULL;
    FILE *fp = NULL;
    char output[256] = {'\0'};
    int ret = 0;

    /* preprocess */
    pp = new_preprocessor();
    preprocess_file(pp, filename);

    if (opt->preprocess) {
        printf("%s", pp->text->buf);
        goto finalize;
    }

    /* parse */
    symtab = new_symbol_table();
    diag = new_diagnostic();

    parser = new_parser();
    tree = parse_text(parser, pp->text->buf, symtab, diag);

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

    /* generate code */
    make_output_filename(filename, output);
    fp = fopen(output, "w");
    if (!fp) {
        ret = 1;
        goto finalize;
    }
    gen_x64(fp, tree, symtab);
    fclose(fp);

finalize:
    free_ast_node(tree);
    free_parser(parser);
    free_diagnostic(diag);
    free_symbol_table(symtab);
    free_preprocessor(pp);

    return ret;
}
