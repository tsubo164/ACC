#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include "position.h"

#define MAX_MESSAGE_COUNT 5

struct message {
    char *str;
    struct position pos;
};

struct diagnostic {
    struct message warnings[MAX_MESSAGE_COUNT];
    struct message errors[MAX_MESSAGE_COUNT];

    int warning_count;
    int error_count;
};

extern struct diagnostic *new_diagnostic();
extern void free_diagnostic(struct diagnostic *diag);

extern void add_warning(struct diagnostic *diag, const struct position *pos,
        const char *msg, ...);
extern void add_error(struct diagnostic *diag, const struct position *pos,
        const char *msg, ...);

extern void print_warnings(const struct diagnostic *diag);
extern void print_errors(const struct diagnostic *diag);

#endif /* _H */
