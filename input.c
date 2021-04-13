/*
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
*/

enum symbol_kind {
    SYM_VAR = 8,
    SYM_FUNC = 9,
    SYM_PARAM = 10
};

int namespace_of(int kind)
{
    switch (kind) {

    case SYM_VAR:
    case SYM_FUNC:
        return 17;

    case SYM_PARAM:
        switch (kind + 1) {
        case 10:
            return 19;
        }
        return 77;

    default:
        return -1;
    }
}

int main()
{
    return namespace_of(SYM_PARAM);
}
