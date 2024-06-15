#ifndef TABLE_H
#define TABLE_H

#include <stdbool.h>

// Table entry struct
typedef struct {
  char* key;
  void* value;
} entry_t;

// Hashtable struct
typedef struct {
  int count;
  int capacity;
  entry_t* entries;
} table_t;

// Creates, initializes, and returns table.
table_t* create_table(void);
// Frees memory used by table.
void free_table(table_t* table);
// Returns value from key in table, NULL if no key found.
void* table_get(table_t* table, const char* key);
// Sets a key value pair in the table.
const char* table_set(table_t* table, const char* key, void* value);
// Prints the table.
void print_table(table_t* table);

#endif
