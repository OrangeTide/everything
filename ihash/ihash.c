/* ihash.c - integer-key hash table */
/* Copyright (c) 2020 jon@rm-f.net
 *
 * Permission to use, copy, modify, or distribute this software for
 * any purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ihash.h"

#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#define ERR (-1)
#define OK (0)

struct ihash_entry {
	uint32_t key;
	void *data; /* if NULL, then entry is empty (and key is ignored) */
};

struct ihash_table {
	unsigned size;
	struct ihash_entry table[];
};

static uint32_t
ihash(uint32_t i)
{
	i = (i + 0x7ed55d16) + (i << 12);
	i = (i ^ 0xc761c23c) ^ (i >> 19);
	i = (i + 0x165667b1) + (i << 5);
	i = (i + 0xd3a2646c) ^ (i << 9);
	i = (i + 0xfd7046c5) + (i << 3);
	i = (i ^ 0xb55a4f09) ^ (i >> 16);

	return i;
}

int
ihash_new(struct ihash_table **table_out, int nelem)
{
	struct ihash_table *table;
	unsigned hashsize, n;

	/* increase size to next prime number */
	hashsize = nelem;
	if (hashsize < 3)
		hashsize = 3;
	hashsize |= 1; /* if even, take next odd */
	while (1) {
		if (hashsize >= UINT_MAX - 2)
			return ERR;

		/* is this prime? */
		for (n = 3; n <= hashsize / n; n += 2)
			if (hashsize % n == 0)
				goto next;
		break; /* passed the prime test */
next:
		hashsize += 2;
	}

	/* allocate */
	table = calloc(1, sizeof(*table) + sizeof(*table->table) * hashsize);
	if (!table)
		return ERR;

	table->size = hashsize;

	if (table_out)
		*table_out = table;
	else
		free(table);

	return OK;
}

void
ihash_free(struct ihash_table *table, void (*freefunc)(void *))
{
	if (!table)
		return;

	/* probe every entry, applying our function */
	if (freefunc) {
		unsigned i, hashsize;

		hashsize = table->size;
		for (i = 0; i < hashsize; i++) {
			if (table->table[i].data) {
				freefunc(table->table[i].data);
				table->table[i].data = NULL;
				table->table[i].key = 0UL;
			}
		}
	}

	free(table);
}

/* generalized search for both lookup and insert */
static struct ihash_entry *
ihash_general_search(struct ihash_table *table, uint32_t key)
{
	uint32_t h, h2;
	unsigned i, orig_i, hashsize = table->size;

	h = ihash(key);
	orig_i = i = h % hashsize + 1;

	struct ihash_entry *e = &table->table[i];

	if (e->data) {
		if (e->key == key) { /* found entry */
			return e;
		}

		h2 = h % (hashsize - 2) + 1; /* second hash value used to modify origional hash */

		do {
			if (i <= h2)
				i = hashsize + i - h2; /* i is less than h2; subtract from hashsize */
			else
				i -= h2;
			if (i == orig_i)
				return NULL; /* failed: visited every node - cannot find or insert */
			e = &table->table[i];

			if (e->key == key) { /* found entry */
				return e;
			}
		} while (e->data);
	}

	/* ran into an empty bucket ... caller can decide to insert or fail search */

	return e;
}

int
ihash_search(struct ihash_table *table, uint32_t key, void **result_out)
{
	static struct ihash_entry *ent;

	ent = ihash_general_search(table, key);

	if (!ent || !ent->data)
		return ERR; /* not found */

	assert(ent->key == key);

	if (result_out)
		*result_out = ent->data;

	return OK;
}

int
ihash_insert(struct ihash_table *table, uint32_t key, void *value)
{
	static struct ihash_entry *ent;

	ent = ihash_general_search(table, key);

	if (!ent) {
		return ERR; /* could not insert */
	}

	if (ent->data) {
		assert(ent->key == key);
		return ERR; /* already an entry at this point */
	}

	ent->key = key;
	ent->data = value;

	return OK;
}
