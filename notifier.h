/* notifier - v0.1.0 - public domain - January 2018 */
#ifndef NOTIFIER_H
#define NOTIFIER_H
#include <stddef.h>

#define NOTIFIER_DONE 0
#define NOTIFIER_OK 1
#define NOTIFIER_STOP -1

struct notifier_block;

typedef int (*notifier_fn_t)(struct notifier_block *nb, unsigned act, void *p);

struct notifier_head {
	struct notifier_block *head;
};

struct notifier_block {
	notifier_fn_t call;
	struct notifier_block *next;
	int priority;
};

static inline void
notifier_head_init(struct notifier_head *nh)
{
	nh->head = NULL;
}

static inline void
notifier_block_init(struct notifier_block *nb, int priority, notifier_fn_t call)
{
	nb->next = NULL;
	nb->priority = priority;
	nb->call = call;
}

static inline
int notifier_register(struct notifier_head *nh, struct notifier_block *nb)
{
	struct notifier_block **cur;

	for (cur = &nh->head; *cur != NULL; cur = &((*cur)->next))
		if (nb->priority > (*cur)->priority)
			break;
	nb->next = *cur;
	*cur = nb;
	return 0;
}

static inline int
notifier_unregister(struct notifier_head *nh, struct notifier_block *nb)
{
	struct notifier_block **cur;

	for (cur = &nh->head; *cur != NULL; cur = &((*cur)->next)) {
		if (*cur == nb) {
			*cur = nb->next;
			nb->next = NULL;
			return 0; /* success */
		}
	}
	return -1; /* no entry found */
}

static inline int
notifier_call(struct notifier_head *nh, unsigned act, void *p)
{
	struct notifier_block *cur, *next;
	int ret;

	for (cur = nh->head; cur; cur = next) {
		next = cur->next;
		ret = cur->call(cur, act, p);
		if (ret <= NOTIFIER_STOP)
			break;
	}
	return ret;
}
#endif
