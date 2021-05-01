#include "proc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static size_t
strlcpy(char *dest, const char *src, size_t size)
{
	size_t i;

	if (size) {
		--size;
		for (i = 0; i < size; i++)
			if (!(dest[i] = src[i]))
				return i;
		dest[i] = 0;
	} else {
		i = 0;
	}
	return i + strlen(src + i);
}

/* used as a dummy callback for when an entry is unitialized */
static int
nothing(struct process *proc __attribute__((unused)))
{
	return -1;
}

////////////////////////////////////////////////////////////////////////

struct process_queue *
proc_queue_new(const char *id)
{
	struct process_queue *q;

	q = calloc(1, sizeof(*q));
	if (!q)
		return NULL;

	STAILQ_INIT(&q->head);
	strlcpy(q->name, id, sizeof(q->name));

	return q;
}

void
proc_queue_free(struct process_queue *q)
{
	struct process *proc;

	if (!q)
		return;

	while ((proc = proc_next(q))) {
		// TODO: some way to free/cancel tasks needs to be added
		fprintf(stderr, "ERROR: leaking process %p\n", proc);
	}

	free(q);
}

int
proc_entry_init(struct process *proc)
{
	if (!proc)
		return -1;

	memset(&proc->proclist, 0, sizeof(proc->proclist));
	proc->run = nothing;

	return 0;
}

int
proc_add(struct process_queue *q, struct process *proc)
{
	if (!q || !proc)
		return -1;

	/* inserting to head could starve other tasks:
	 * STAILQ_INSERT_HEAD(&q->head, proc, proclist);
	 * instead do this:
	 */
	STAILQ_INSERT_TAIL(&q->head, proc, proclist);

	return 0;
}

struct process *
proc_next(struct process_queue *q)
{
	struct process *proc;

	proc = STAILQ_FIRST(&q->head);
	if (proc)
		STAILQ_REMOVE_HEAD(&q->head, proclist);

	return proc;
}

/* runs next entry in queue,
 * returns ran entry and updates result_out with return value */
struct process *
proc_run(struct process_queue *q, int *result_out)
{
	struct process *proc = proc_next(q);

	if (!proc)
		return NULL;

	int result = proc->run(proc);
	if (result_out)
		*result_out = result;

	return proc;
}

void
proc_set_run(struct process *proc, int (*run)(struct process *))
{
	proc->run = run ? run : nothing;
}
