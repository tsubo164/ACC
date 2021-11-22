#ifndef __ASSERT_H
#define __ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#define assert(expr) \
    (expr) ? ((void)0) : fprintf(stderr, "Assertion failed: (expr)\n"), abort()

#endif /* __ASSERT_H */
