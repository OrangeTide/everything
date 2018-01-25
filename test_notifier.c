/* test_notifier - test for notifier.h - public domain. */
#include <stdlib.h>
#include <stdio.h>
#include "container_of.h"
#include "notifier.h"

enum {
	EXAMPLE_ACT_PRINT,
	EXAMPLE_ACT_FREE,
};

struct example {
	struct notifier_block notifier;
	int foo;
};

static void
example_free(struct example *n)
{
	free(n);
}

static int
mycall(struct notifier_block *nb, unsigned act, void *p)
{
	struct example *n = container_of(nb, struct example, notifier);
	switch (act) {
	case EXAMPLE_ACT_PRINT:
		fprintf(stderr, "%p:PRINT:foo=%d\n", n, n->foo);
		break;
	case EXAMPLE_ACT_FREE:
		fprintf(stderr, "%p:FREE\n", n);
		example_free(n);
		break;
	};
	return NOTIFIER_OK;
}

static struct example *
example_new(int foo)
{
	struct example *n;

	n = calloc(1, sizeof(*n));
	if (!n)
		return NULL;
	n->foo = foo;
	notifier_block_init(&n->notifier, 0, mycall);
	fprintf(stderr, "%p:ALLOC\n", n);
	return n;
}

int
main()
{
	struct notifier_head head;
	int i;

	notifier_head_init(&head);

	for (i = 0; i < 8; i++) {
		struct example *n;
		n = example_new(i * 6);
		notifier_register(&head, &n->notifier);
	}

	notifier_call(&head, EXAMPLE_ACT_PRINT, NULL);

	notifier_call(&head, EXAMPLE_ACT_FREE, NULL);

	return 0;
}
