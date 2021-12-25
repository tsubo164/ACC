#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "string_table.h"

static unsigned int hash_fn(const char *key)
{
    unsigned int h = 0;
    unsigned const char *p = NULL;

    for (p = (unsigned const char *) key; *p != '\0'; p++)
        h = MULTIPLIER * h + *p;

    return h % HASH_SIZE;
}

static struct table_entry *new_entry(const char *src)
{
    struct table_entry *ent = malloc(sizeof(struct table_entry));
    const size_t alloc = strlen(src) + 1;
    char *dst = malloc(sizeof(char) * alloc);

    strncpy(dst, src, alloc);
    ent->str = dst;
    ent->next = NULL;

    return ent;
}

static void free_entry(struct table_entry *entry)
{
    if (!entry)
        return;

    free(entry->str);
    free(entry);
}

struct string_table *new_string_table()
{
    struct string_table *table = malloc(sizeof(struct string_table));

    init_string_table(table);

    return table;
}

void free_string_table(struct string_table *table)
{
    int i;

    if (!table)
        return;

    for (i = 0; i < HASH_SIZE; i++) {
        struct table_entry *ent = table->entries[i], *tmp;
        if (!ent)
            continue;

        while (ent) {
            tmp = ent->next;
            free_entry(ent);
            ent = tmp;
        }
    }

    free(table);
}

void init_string_table(struct string_table *table)
{
    int i;
    for (i = 0; i < HASH_SIZE; i++)
        table->entries[i] = NULL;
}

const char *insert_string(struct string_table *table, const char *src)
{
    struct table_entry *ent;
    const unsigned int h = hash_fn(src);

    for (ent = table->entries[h]; ent; ent = ent->next)
        if (!strcmp(src, ent->str))
            return ent->str;

    ent = new_entry(src);
    ent->next = table->entries[h];
    table->entries[h] = ent;

    return ent->str;
}
