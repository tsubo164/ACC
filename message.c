#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "message.h"

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
    msg->str = NULL; 
    msg->file_pos = 0;
}

struct message_list *new_message_list()
{
    struct message_list *list = malloc(sizeof(struct message_list));
    int i;

    for (i = 0; i < MAX_MESSAGE_COUNT; i++) {
        init_message(&list->warnings[i]);
        init_message(&list->errors[i]);
    }

    list->warning_count = 0;
    list->error_count = 0;
    list->strtab = NULL;

    return list;
}

void free_message_list(struct message_list *list)
{
    free(list);
}

void add_warning(struct message_list *list, const struct position *pos,
        const char *msg, ...)
{
    va_list va;
    va_start(va, msg);
    {
        struct message *m;
        char buf[1024] = {'\0'};

        if (list->warning_count >= MAX_MESSAGE_COUNT) {
            list->warning_count++;
            return;
        }

        m = &list->warnings[list->warning_count++];
        m->pos = *pos;

        vsprintf(buf, msg, va);
        m->str = insert_string(list->strtab, buf);
    }
    va_end(va);
}

void add_error(struct message_list *list, const struct position *pos,
        const char *msg, ...)
{
    va_list va;
    va_start(va, msg);
    {
        struct message *m;
        char buf[1024] = {'\0'};

        if (list->error_count >= MAX_MESSAGE_COUNT) {
            list->error_count++;
            return;
        }

        m = &list->errors[list->error_count++];
        m->pos = *pos;

        vsprintf(buf, msg, va);
        m->str = insert_string(list->strtab, buf);
    }
    va_end(va);
}

void print_warning_messages(const struct message_list *list)
{
    print_message_array(list->warnings, WARNING);
}

void print_error_messages(const struct message_list *list)
{
    print_message_array(list->errors, ERROR);
}
