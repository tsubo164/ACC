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
    struct table_entry *entry = malloc(sizeof(struct table_entry));
    const size_t alloc = strlen(src) + 1;
    char *dst = malloc(sizeof(char) * alloc);

    strncpy(dst, src, alloc);
    entry->str = dst;
    entry->next = NULL;

    return entry;
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
    struct table_entry *entry = NULL;
    struct table_entry *kill = NULL;
    int i;

    if (!table)
        return;

    for (i = 0; i < HASH_SIZE; i++) {
        if (!table->entries[i])
            continue;

        for (entry = table->entries[i]; entry != NULL; ) {
            kill = entry;
            entry = entry->next;
            free_entry(kill);
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
    struct table_entry *entry = NULL;
    const unsigned int h = hash_fn(src);

    for (entry = table->entries[h]; entry != NULL; entry = entry->next) {
        if (!strcmp(src, entry->str))
            return entry->str;
    }

    entry = new_entry(src);

    entry->next = table->entries[h];
    table->entries[h] = entry;

    return entry->str;
}
