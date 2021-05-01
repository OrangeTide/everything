#include "arena.h"
#include "arena.c" /* embed implementation into this unit test */

#include <stdio.h>

struct test {
	int run_count;
	struct arena *arena;
};

static void
test_free(struct test *t)
{
	if (t) {
		arena_free(t->arena);
		free(t);
	}
}

static struct test *
test_new(void)
{
	struct test *t = calloc(1, sizeof(*t));

	if (!t)
		return NULL;

	struct arena *a = arena_new();
	if (!a) {
		test_free(t);
		return NULL;
	}

	t->arena = a;

	return t;
}

static int
test_run(struct test *t)
{
	t->run_count++;

	return OK;
}

int
main()
{
	/******/

	int result = OK;

	struct test *T = test_new();
	if (!T)
		return 1;

	if (test_run(T) != OK)
		result = ERR;

	test_free(T);

	/******/

	if (result != OK) {
		puts("Test failed!");
		return 1;
	} else {
		puts("Test passed!");
		return 0;
	}
}
