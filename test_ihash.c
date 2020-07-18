/* test_ihash.c - test for ihash.c */
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

#include "ihash.c"

#include <stdio.h>
#include <string.h>

#define TEST_FUNC(cond, expected, description) do { \
	if ((cond) == (expected)) { \
		fprintf(stderr, "PASS: %s\n", (description)); \
	} else { \
		fprintf(stderr, "FAIL: %s (%s)\n", (description), #cond); \
	} \
	} while(0)

#define TEST_REPORT(description) do { \
		fprintf(stderr, "INFO: %s\n", (description)); \
	} while(0)

static void
free_string(void *s)
{
	free(s);
}

/* test1 allocates a new entry for each test vector */
static int
test1_load(struct ihash_table *table)
{
	unsigned i;
	const struct { uint32_t key; char *s; } t1_set[] = {
		{ 44, "fourty-four" },
		{ 1000, "one thousand" },
		{ 4, "four" },
		{ 18, "eighteen" },
		{ 101, "one hundred one" },
		{ 0, "zero" },
		{ 0xff00ff00, "a large number" },
	};

	for (i = 0; i < sizeof(t1_set) / sizeof(*t1_set); i++) {
		int result;

		char *s = strdup(t1_set[i].s);
		result = ihash_insert(table, t1_set[i].key, s);

		if (result != OK) {
			free(s);
			return ERR;
		}
	}

	return OK;
}

const struct { uint32_t key; const char *s; } t2_set[] = {
	{ 0x00000010u, "cat" },
	{ 0x00000000u, "dog" },
	{ 0x00010000u, "horse" },
	{ 0x10000000u, "goat" },
	{ 0x90000000u, "pelican" },
	{ 0x00300000u, "robin" },
	{ 0x33333333u, "zebra" },
	{ 0x55555555u, "aardvark" },
	{ 0xaaaaaaaau, "salamander" },
};

/* test2 points directly at the embedded data without allocating */
static int
test2_load(struct ihash_table *table)
{
	unsigned i;
	for (i = 0; i < sizeof(t2_set) / sizeof(*t2_set); i++) {
		int result;

		result = ihash_insert(table, t2_set[i].key, (void*)t2_set[i].s);

		if (result != OK) {
			return ERR;
		}
	}

	return OK;
}

/* try to insert some entries again, expecting them to fail */
static int
test2_negative(struct ihash_table *table)
{
	unsigned i;
	for (i = 1; i < sizeof(t2_set) / sizeof(*t2_set); i += 2) {
		int result;

		result = ihash_insert(table, t2_set[i].key, (void*)t2_set[i].s);

		if (result == OK) {
			return ERR;
		}
	}

	return OK;
}

int
main()
{
	struct ihash_table *t1, *t2;

	TEST_FUNC(ihash_new(&t1, 3000), OK, "create small hash table");
	TEST_FUNC(ihash_new(&t2, 80000), OK, "create big hash table");

	fprintf(stderr, "t1 size = %d\n", t1->size);
	fprintf(stderr, "t2 size = %d\n", t2->size);

	TEST_FUNC(test1_load(t1), OK, "load small hash table");
	TEST_FUNC(test2_load(t2), OK, "load big hash table");

	TEST_FUNC(test2_negative(t2), OK, "duplicate insert into big hash table");

	ihash_free(t1, free_string);
	TEST_REPORT("free'd small hash table");

	ihash_free(t2, NULL);
	TEST_REPORT("free'd big hash table");

	return 0;
}
