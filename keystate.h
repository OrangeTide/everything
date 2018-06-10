/* keystate.h : keyboard and gamepad button state - public domain. */
#ifndef KEYSTATE_H
#define KEYSTATE_H

/* Theory of operation
 *
 * The event framework that receieves key events uses keystate_send() on a
 * handle to publish that state is up or down.
 *
 * keystate_register() or keystate_connect() is used to create a handle by
 * name. These two functions are similar except keystate_connect() returns NULL
 * if no such name exists, and is added as an additional safety check for typos
 * on the subscribed named.
 *
 * Exactly one file in your project must defined KEYSTATE_IMPLEMENTATION before
 * including this header.
 *
 * keystate_free() can be used to deallocate the global table. This has the
 * consequence of invalidating all outstanding handles.
 *
 * keystate_put() takes a reference on an entry. keystate_get() releases a
 * reference of an entry.
 *
 */

typedef struct keystate { unsigned char down; } keystate;

void keystate_free(void);
keystate *keystate_register(const char *id);
keystate *keystate_connect(const char *id);
void keystate_put(keystate *ks);
void keystate_get(keystate *ks);

static inline void
keystate_send(keystate *ks, int down)
{
	if (ks)
		ks->down = down;
}

static inline int
keystate_is_down(keystate *ks)
{
	return ks ? ks->down : 0;
}

#ifdef KEYSTATE_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct { keystate ks; char id[16]; unsigned rc; } keystate_entry;

static keystate_entry **keystate_table;
static unsigned keystate_table_len, keystate_table_max;

void
keystate_put(keystate *ks)
{
	keystate_entry *e = (keystate_entry*)((char*)ks - offsetof(keystate_entry, ks));

	if (!e->rc) {
		free(e);
	}

	e->rc--;
}

void keystate_get(keystate *ks)
{
	keystate_entry *e = (keystate_entry*)((char*)ks - offsetof(keystate_entry, ks));

	e->rc++;
}

// TODO: would be nice to remove these small dynamic allocations
static keystate_entry *keystate_entry_alloc(const char *id)
{
	keystate_entry *e;

	e = calloc(1, sizeof(*e));
	strncpy(e->id, id, sizeof(e->id));
	e->ks.down = 0;
	e->rc = 0;

	return e;
}

void
keystate_free(void)
{
	unsigned i;

	for (i = 0; i < keystate_table_len; i++) {
		if (keystate_table[i])
			keystate_put(&keystate_table[i]->ks);
		keystate_table[i] = NULL;
	}

	free(keystate_table);
	keystate_table = NULL;
	keystate_table_len = keystate_table_max = 0;
}

static int
keystate_table_grow(unsigned size)
{
	keystate_entry **t;
	size_t oldsz = keystate_table_max * sizeof(*t),
	       newsz = size * sizeof(*t);

	t = realloc(keystate_table, newsz);
	if (!t)
		return 0;

	if (newsz > oldsz)
		memset(t + keystate_table_max, 0, newsz - oldsz);
	keystate_table = t;
	keystate_table_max = size;

	return 1;
}

static keystate_entry *
keystate_lookup(const char *id)
{
	unsigned i;

	// TODO: use something better than a linear probe
	for (i = 0; i < keystate_table_len; i++) {
		keystate_entry **p = keystate_table + i;
		if (*p && !strncmp((*p)->id, id, sizeof((*p)->id)))
			return *p;
	}

	return NULL;
}

keystate *
keystate_register(const char *id)
{
	keystate_entry *e, **slot;

	e = keystate_lookup(id);
	if (!e) {
		/* check that there is room in the table for another entry */
		if (keystate_table_len >= keystate_table_max) {
			if (!keystate_table_grow(keystate_table_len ?
						keystate_table_len * 2 : 1))
				return 0;
		}

		/* add a new entry to end of table */
		slot = keystate_table + keystate_table_len++;

		e = keystate_entry_alloc(id);
		*slot = e;
	}

	keystate_get(&e->ks);
	return &e->ks;
}

keystate *
keystate_connect(const char *id)
{
	keystate_entry *e;

	e = keystate_lookup(id);
	if (!e)
		return NULL;

	keystate_get(&e->ks);
	return &e->ks;
}

#endif /* KEYSTATE_IMPLEMENTATION */

#endif
