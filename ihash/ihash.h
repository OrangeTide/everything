/* ihash.h - integer-key hash table. */
#ifndef IHASH_H_
#define IHASH_H_
#include <stdint.h>

struct ihash_table;

int ihash_new(struct ihash_table **table_out, int nelem);
void ihash_free(struct ihash_table *table, void (*freefunc)(void *));
int ihash_search(struct ihash_table *table, uint32_t key, void **result_out);
int ihash_insert(struct ihash_table *table, uint32_t key, void *value);
#endif
