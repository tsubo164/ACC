#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "diagnostic.h"

#define TERMINAL_COLOR_BLACK   "\x1b[30m"
#define TERMINAL_COLOR_RED     "\x1b[31m"
#define TERMINAL_COLOR_GREEN   "\x1b[32m"
#define TERMINAL_COLOR_YELLOW  "\x1b[33m"
#define TERMINAL_COLOR_BLUE    "\x1b[34m"
#define TERMINAL_COLOR_MAGENTA "\x1b[35m"
#define TERMINAL_COLOR_CYAN    "\x1b[36m"
#define TERMINAL_COLOR_WHITE   "\x1b[37m"
#define TERMINAL_COLOR_RESET   "\x1b[39m"

#define TERMINAL_DECORATION_BOLD    "\x1b[1m"
#define TERMINAL_DECORATION_RESET   "\x1b[0m"

enum {
    WARNING,
    ERROR
};

static void print_line(const char *filepath, int row, int col)
{
    FILE *fp;
    int x = 0, y = 1;

    fp = fopen(filepath, "r");
    if (!fp) {
        /* error */
        return;
    }

    for (;;) {
        int c;

        if (y == row)
            break;

        c = fgetc(fp);

        if (c == EOF)
            break;

        if (c == '\n')
            y++;
    }

    for (;;) {
        const int c = fgetc(fp);
        fprintf(stderr, "%c", c);

        if (c == '\n' || c == EOF)
            break;
    }

    for (x = 0; x < col - 1; x++) {
        fprintf(stderr, " ");
    }

    fprintf(stderr, TERMINAL_DECORATION_BOLD);
        fprintf(stderr, TERMINAL_COLOR_GREEN);
            fprintf(stderr, "^");
        fprintf(stderr, TERMINAL_COLOR_RESET);
    fprintf(stderr, TERMINAL_DECORATION_RESET);
    fprintf(stderr, "\n");

    fclose(fp);
}

static void print_message(const struct message *msg, int msg_type)
{
    const char *err_filepath = msg->pos.filename;
    const int err_col = msg->pos.x;
    const int err_row = msg->pos.y;

    if (msg_type == ERROR) {
        fprintf(stderr, TERMINAL_DECORATION_BOLD);
            fprintf(stderr, "%s:", err_filepath);
            fprintf(stderr, "%d:%d: ", err_row, err_col);
            fprintf(stderr, TERMINAL_COLOR_RED);
                fprintf(stderr, "%s: ", "error");
            fprintf(stderr, TERMINAL_COLOR_RESET);
            fprintf(stderr, "%s\n", msg->str);
        fprintf(stderr, TERMINAL_DECORATION_RESET);
    }
    if (msg_type == WARNING) {
        fprintf(stderr, TERMINAL_DECORATION_BOLD);
            fprintf(stderr, "%s:", err_filepath);
            fprintf(stderr, "%d:%d: ", err_row, err_col);
            fprintf(stderr, TERMINAL_COLOR_MAGENTA);
                fprintf(stderr, "%s: ", "warning");
            fprintf(stderr, TERMINAL_COLOR_RESET);
            fprintf(stderr, "%s\n", msg->str);
        fprintf(stderr, TERMINAL_DECORATION_RESET);
    }
    print_line(err_filepath, err_row, err_col);
}

static void print_message_array(const struct message *msg_array, int msg_type)
{
    int i;

    for (i = 0; i < MAX_MESSAGE_COUNT; i++) {
        const struct message *msg = &msg_array[i];

        if (msg->str)
            print_message(msg, msg_type);
    }
}

static void init_message(struct message *msg)
{
    if (!msg)
        return;
    msg->str = NULL; 
}

static void free_message(struct message *msg)
{
    if (!msg)
        return;
    free(msg->str);
}

struct diagnostic *new_diagnostic()
{
    struct diagnostic *diag = malloc(sizeof(struct diagnostic));
    int i;

    for (i = 0; i < MAX_MESSAGE_COUNT; i++) {
        init_message(&diag->warnings[i]);
        init_message(&diag->errors[i]);
    }

    diag->warning_count = 0;
    diag->error_count = 0;

    return diag;
}

void free_diagnostic(struct diagnostic *diag)
{
    int i;

    if (!diag)
        return;

    for (i = 0; i < MAX_MESSAGE_COUNT; i++) {
        free_message(&diag->warnings[i]);
        free_message(&diag->errors[i]);
    }

    free(diag);
}

void add_warning(struct diagnostic *diag, const struct position *pos,
        const char *msg, ...)
{
    va_list va;
    va_start(va, msg);
    {
        struct message *m;
        char buf[1024] = {'\0'};

        if (diag->warning_count >= MAX_MESSAGE_COUNT) {
            diag->warning_count++;
            return;
        }

        m = &diag->warnings[diag->warning_count++];
        m->pos = *pos;

        vsprintf(buf, msg, va);
        m->str = malloc(sizeof(char) * (strlen(buf) + 1));
        strcpy(m->str, buf);
    }
    va_end(va);
}

void add_error(struct diagnostic *diag, const struct position *pos,
        const char *msg, ...)
{
    va_list va;
    va_start(va, msg);
    {
        struct message *m;
        char buf[1024] = {'\0'};

        if (diag->error_count >= MAX_MESSAGE_COUNT) {
            diag->error_count++;
            return;
        }

        m = &diag->errors[diag->error_count++];
        m->pos = *pos;

        vsprintf(buf, msg, va);
        m->str = malloc(sizeof(char) * (strlen(buf) + 1));
        strcpy(m->str, buf);
    }
    va_end(va);
}

void print_warnings(const struct diagnostic *diag)
{
    print_message_array(diag->warnings, WARNING);
}

void print_errors(const struct diagnostic *diag)
{
    print_message_array(diag->errors, ERROR);
}
