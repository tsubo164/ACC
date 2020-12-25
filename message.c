#include <stdio.h>
#include <stdlib.h>
/* TODO remove this */
#include <string.h>
#include "message.h"

#define MEM_ALLOC(type) ((type *) malloc(sizeof(type)))
#define MEM_FREE(ptr) free(ptr)

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

static void print_message(FILE *fp, const char *filename,
        const struct message *msg, const char *msg_type)
{
    const long target_pos = msg->file_pos;
    int found_target_location = 0;
    int err_col, err_row;
    int x = 0, y = 0; /* scanning pos row and column */
    int c = '\0';
    char line[1024] = {'\0'};

    fseek(fp, 0L, SEEK_SET);

    for (;;) {
        c = fgetc(fp);

        if (ftell(fp) == target_pos) {
            found_target_location = 1;
            err_col = x + 1; /* x starts with 0 */
            err_row = y + 1;
        }

        if (c == '\n' || c == EOF) {
            if (found_target_location) {
                line[x] = '\0';
                break;
            } else {
                y++;
                x = 0;
                line[x] = '\0';
            }
        } else {
            line[x++] = c;
        }
    }

    fprintf(stderr, TERMINAL_DECORATION_BOLD);
        fprintf(stderr, "%s:", filename);
        fprintf(stderr, "%d:%d: ", err_row, err_col);
        fprintf(stderr, TERMINAL_COLOR_RED);
            fprintf(stderr, "%s: ", msg_type);
        fprintf(stderr, TERMINAL_COLOR_RESET);
        fprintf(stderr, "%s\n", msg->str);
    fprintf(stderr, TERMINAL_DECORATION_RESET);

    fprintf(stderr, "%s\n", line);
    fprintf(stderr, "%*s", err_col - 1, ""); /* nspaces = col - 1 */

    fprintf(stderr, TERMINAL_DECORATION_BOLD);
    fprintf(stderr, TERMINAL_COLOR_GREEN);
        fprintf(stderr, "%c\n", '^');
    fprintf(stderr, TERMINAL_COLOR_RESET);
    fprintf(stderr, TERMINAL_DECORATION_RESET);
}

static void print_message2(FILE *fp, const char *filename,
        const struct message *msg, const char *msg_type)
{
    /*
    const long target_pos = msg->file_pos;
    int found_target_location = 0;
    */
    int err_col = 0, err_row = 0;
#if 0
    int x = 0, y = 0; /* scanning pos row and column */
    int c = '\0';
    char line[1024] = {'\0'};
#endif
    err_col = msg->pos.x,
    err_row = msg->pos.y;

    if (!strcmp(msg_type, "error")) {
        fprintf(stderr, TERMINAL_DECORATION_BOLD);
            fprintf(stderr, "%s:", filename);
            fprintf(stderr, "%d:%d: ", err_row, err_col);
            fprintf(stderr, TERMINAL_COLOR_RED);
                fprintf(stderr, "%s: ", msg_type);
            fprintf(stderr, TERMINAL_COLOR_RESET);
            fprintf(stderr, "%s\n", msg->str);
        fprintf(stderr, TERMINAL_DECORATION_RESET);
    }
    if (!strcmp(msg_type, "warning")) {
        fprintf(stderr, TERMINAL_DECORATION_BOLD);
            fprintf(stderr, "%s:", filename);
            fprintf(stderr, "%d:%d: ", err_row, err_col);
            fprintf(stderr, TERMINAL_COLOR_MAGENTA);
                fprintf(stderr, "%s: ", msg_type);
            fprintf(stderr, TERMINAL_COLOR_RESET);
            fprintf(stderr, "%s\n", msg->str);
        fprintf(stderr, TERMINAL_DECORATION_RESET);
    }

#if 0
    fprintf(stderr, "%s\n", line);
    fprintf(stderr, "%*s", err_col - 1, ""); /* nspaces = col - 1 */

    fprintf(stderr, TERMINAL_DECORATION_BOLD);
    fprintf(stderr, TERMINAL_COLOR_GREEN);
        fprintf(stderr, "%c\n", '^');
    fprintf(stderr, TERMINAL_COLOR_RESET);
    fprintf(stderr, TERMINAL_DECORATION_RESET);
#endif
}

static void print_message_array(FILE *fp, const char *filename,
        const struct message *msg_array, const char *msg_type)
{
    int i;

    for (i = 0; i < MAX_MESSAGE_COUNT; i++) {
        const struct message *msg = &msg_array[i];

        if (msg->str) {
            if (0)
                print_message(fp, filename, msg, msg_type);
            print_message2(fp, filename, msg, msg_type);
        }
    }
}

static void init_message(struct message *msg)
{
    msg->str = NULL; 
    msg->file_pos = 0;
}

struct message_list *new_message_list()
{
    struct message_list *list = MEM_ALLOC(struct message_list);
    int i;

    for (i = 0; i < MAX_MESSAGE_COUNT; i++) {
        init_message(&list->warnings[i]);
        init_message(&list->errors[i]);
    }

    list->warning_count = 0;
    list->error_count = 0;

    return list;
}

void free_message_list(struct message_list *list)
{
    MEM_FREE(list);
}

void add_warning(struct message_list *list, const char *msg, const struct position *pos)
{
    struct message *m;

    if (list->warning_count >= MAX_MESSAGE_COUNT) {
        list->warning_count++;
        return;
    }

    m = &list->warnings[list->warning_count++];
    m->str = msg;
    m->pos = *pos;
}

void add_error(struct message_list *list, const char *msg, const struct position *pos)
{
    struct message *m;

    if (list->error_count >= MAX_MESSAGE_COUNT) {
        list->error_count++;
        return;
    }

    m = &list->errors[list->error_count++];
    m->str = msg;
    m->pos = *pos;
}

void print_warning_messages(FILE *fp, const char *filename,
        const struct message_list *list)
{
    print_message_array(fp, filename, list->warnings, "warning");
}

void print_error_messages(FILE *fp, const char *filename,
        const struct message_list *list)
{
    print_message_array(fp, filename, list->errors, "error");
}
