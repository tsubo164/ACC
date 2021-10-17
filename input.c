/*
#include <stdlib.h>
#include <string.h>
*/
#include <stdio.h>

struct foo {
    /*
    int i;
    char *c;
    */
    char i;
};

struct bar {
    /*
    int i;
    struct foo f;
    */
    short i;
    struct foo f;
};

struct point {
    int x, y, z;
};

typedef struct C {
    char a;
} C;

typedef struct S {
    short a;
} S;

typedef struct SC {
    short a;
    char b;
} SC;

typedef struct I {
    int a;
} I;

typedef struct IC {
    int i;
    char a;
} IC;

typedef struct ICI {
    int i;
    char a;
    int j;
} ICI;

typedef struct ICS {
    int i;
    char a;
    short j;
} ICS;

typedef struct PC {
    void *p;
    char a;
} PC;

int main()
{
    struct bar b;
    int a;

    printf("--- %p\n", (void *) &b.f.i);
    printf("--- %p\n", (void *) &b.i);
    printf("--- sizeof(foo):   %ld\n", sizeof(struct foo));
    printf("--- sizeof(point): %ld\n", sizeof(struct point));

    printf("==================================\n");
    printf("--- sizeof(C):    %ld\n", sizeof(C));
    printf("--- sizeof(S):    %ld\n", sizeof(S));
    printf("--- sizeof(SC):   %ld\n", sizeof(SC));
    printf("--- sizeof(I):    %ld\n", sizeof(I));
    printf("--- sizeof(IC):   %ld\n", sizeof(IC));
    printf("--- sizeof(ICI):  %ld\n", sizeof(ICI));
    printf("--- sizeof(ICS):  %ld\n", sizeof(ICS));
    printf("--- sizeof(PC):   %ld\n", sizeof(PC));

    {
        C c2[2];

        printf("--- sizeof(c2):   %ld\n", sizeof(c2));
    }
    {
        ICI ici2[2];

        printf("--- sizeof(c2):   %ld\n", sizeof(ici2));
    }
    {
        struct FOO {
            int a;
            struct BAR {
                int b;
            } b;
        };

        printf("******** %ld\n", sizeof(struct FOO));
    }

    return 13;
}
