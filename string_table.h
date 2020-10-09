#ifndef STRING_TABLE_H
#define STRING_TABLE_H

#define HASH_SIZE 1237 /* a prime number */
#define MULTIPLIER 31

struct table_entry {
	char *str;
	struct table_entry *next;
};

struct string_table {
	struct table_entry *entries[HASH_SIZE];
};

extern struct string_table *new_string_table();
extern void free_string_table(struct string_table *table);

extern void init_string_table(struct string_table *table);
extern const char *insert_string(struct string_table *table, const char *src);

extern int str_match(const char *a, const char *b);

#endif /* _H */
