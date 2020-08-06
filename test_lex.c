#include <stdio.h>
#include <string.h>
#include "lexer.h"

static int n_pass = 0;
static int n_fail = 0;
static int n_total = 0;

#define TEST_INT(a, b) \
    do { if ((a)==(b)) {\
        test_pass( #a" == "#b, __FILE__, __LINE__ ); \
    } else { \
        test_fail( #a" == "#b, __FILE__, __LINE__ ); \
        printf("  actual:   %d\n", (a)); \
        printf("  expected: %d\n", (b)); \
    } } while (0)

#define TEST_LONG(a, b) \
    do { if ((a)==(b)) {\
        test_pass( #a" == "#b, __FILE__, __LINE__ ); \
    } else { \
        test_fail( #a" == "#b, __FILE__, __LINE__ ); \
        printf("  actual:   %ld\n", (a)); \
        printf("  expected: %ld\n", (b)); \
    } } while (0)

#define TEST_STRING(a, b) \
    do { if (!strcmp(a,b)) {\
        test_pass( #a" == "#b, __FILE__, __LINE__ ); \
    } else { \
        test_fail( #a" == "#b, __FILE__, __LINE__ ); \
        printf("  actual:   %s\n", a); \
        printf("  expected: %s\n", b); \
    } } while (0)

#define TEST_REPORT() \
    printf("%s: %d/%d/%d: (fail/pass/total)\n", __FILE__, \
    test_get_fail_count(), test_get_pass_count(), test_get_total_count());

void test_pass(const char *expr, const char *file, int line)
{
    /*
    fprintf(stdout, "  :PASS :%s:%d: %s\n", file, line, expr);
    */
    n_pass++;
    n_total++;
}

void test_fail(const char *expr, const char *file, int line)
{
    fprintf(stdout, "fail: %s:%d: %s\n", file, line, expr);
    n_fail++;
    n_total++;
}

int test_get_pass_count()
{
    return n_pass;
}

int test_get_fail_count()
{
    return n_fail;
}

int test_get_total_count()
{
    return n_total;
}

int main()
{
    const char *filename = "test_lex_in.c";
    {
        FILE *f = fopen(filename, "w");
        fprintf(f, " a = 1 < \n ; 12 38234 7 ab_ _xyz123  return ");
        fclose(f);
    }
    {
        FILE *f = fopen(filename, "r");
        struct lexer lex;
        struct token tok;

        lexer_init(&lex);
        token_init(&tok);
        lex.file = f;

        TEST_LONG(token_file_pos(&tok), 0L);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, TK_IDENT);
        TEST_STRING(tok.word, "a");
        TEST_LONG(token_file_pos(&tok), 1L);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, '=');
        TEST_INT(tok.value, 0);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, TK_NUM);
        TEST_INT(tok.value, 1);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, '<');
        TEST_INT(tok.value, 0);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, ';');
        TEST_INT(tok.value, 0);
        TEST_LONG(token_file_pos(&tok), 11L);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, TK_NUM);
        TEST_INT(tok.value, 12);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, TK_NUM);
        TEST_INT(tok.value, 38234);
        TEST_LONG(token_file_pos(&tok), 16L);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, TK_NUM);
        TEST_INT(tok.value, 7);
        TEST_LONG(token_file_pos(&tok), 22L);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, TK_IDENT);
        TEST_STRING(tok.word, "ab_");

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, TK_IDENT);
        TEST_STRING(tok.word, "_xyz123");

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, TK_RETURN);

        lex_get_token(&lex, &tok);
        TEST_INT(tok.kind, TK_EOF);
        TEST_INT(tok.value, 0);

        fclose(f);
    }

    TEST_REPORT();
    return 0;
}
