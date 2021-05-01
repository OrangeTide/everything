#ifndef ARENA_H_
#define ARENA_H_

#include <stddef.h>

struct arena;

struct arena *arena_new(void);
void arena_free(struct arena *a);
int arena_resize(struct arena **arena_inout, size_t new_size);
int arena_increase(struct arena **arena_inout, size_t amount);
void *arena_data(struct arena *a, int offset, size_t len);

#endif
