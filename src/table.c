#include "table.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

#define INITIAL_CAPACITY 16

// Function to hash a key.
static uint64_t hash_key(const char* key) {
  uint64_t hash = FNV_OFFSET;
  for (const char* p = key; *p; p++) {
    hash ^= (uint64_t)(unsigned char)(*p);
    hash *= FNV_PRIME;
  }
  return hash;
}

// Creates and initializes table.
table_t* create_table(void) {
  table_t* table = malloc(sizeof(table_t));
  if (table == NULL) {
    return NULL;
  }
  table->count = 0;
  table->capacity = INITIAL_CAPACITY;

  table->entries = calloc(table->capacity, sizeof(entry_t));
  if (table->entries == NULL) {
    free(table);
    return NULL;
  }
  return table;
}

// Frees the memory used by the table and the table itself.
void free_table(table_t* table) {
  for (int i = 0; i < table->capacity; i++) {
    free((void*)table->entries[i].key);
  }

  free(table->entries);
  free(table);
}

// Returns value specified by key, NULL if there is no key.
void* table_get(table_t* table, const char* key) {
  uint64_t hash = hash_key(key);
  size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

  while (table->entries[index].key != NULL) {
    if (strcmp(key, table->entries[index].key) == 0) {
      return table->entries[index].value;
    }
    index++;
    if (index >= table->capacity) {
      index = 0;
    }
  }
  return NULL;
}

// Sets entry inside of table.
static const char* table_set_entry(entry_t* entries, int capacity,
                                   const char* key, void* value, int* plength) {
  // Hash the key.
  uint64_t hash = hash_key(key);
  // Find index.
  size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

  while (entries[index].key != NULL) {
    // Find next bucket.
    if (strcmp(key, entries[index].key) == 0) {
      entries[index].value = value;
      return entries[index].key;
    }
    index++;
    if (index >= capacity) {
      index = 0;
    }
  }

  if (plength != NULL) {
    key = strdup(key);
    if (key == NULL) {
      return NULL;
    }
    (*plength)++;
  }
  entries[index].key = (char*)key;
  entries[index].value = value;
  return key;
}

// Helper to expand the table capacity and reallocate the memory.
static bool table_expand(table_t* table) {
  int new_capacity = table->capacity * 2;
  if (new_capacity < table->capacity) {
    return false;
  }

  // Calloc to initialize new entries.
  entry_t* new_entries = calloc(new_capacity, sizeof(entry_t));
  if (new_entries == NULL) {
    return false;
  }

  // Copy old entries over.
  for (int i = 0; i < table->capacity; i++) {
    entry_t entry = table->entries[i];
    if (entry.key != NULL) {
      table_set_entry(new_entries, new_capacity, entry.key,
                      entry.value, NULL);
    }
  }

  // Free old entries and update to new entries.
  free(table->entries);
  table->entries = new_entries;
  table->capacity = new_capacity;
  return true;
}

// Sets a key value pair in the table.
const char* table_set(table_t* table, const char* key, void* value) {
  assert(value != NULL);
  if (value == NULL) {
    return NULL;
  }

  // Expand table if needed.
  if (table->count >= table->capacity / 2) {
    if (!table_expand(table)) {
      return NULL;
    }
  }

  return table_set_entry(table->entries, table->capacity, key, value,
                         &table->count);
}

// Prints the table's entries' keys and values.
void print_table(table_t* table) {
  for (int i = 0; i < table->capacity; i++) {
    if (table->entries[i].key != NULL) {
      printf("Key: %s, Value: %s\n", table->entries[i].key, (char*)table->entries[i].value);
    }
  }
}
