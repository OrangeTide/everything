#ifndef PROC_H_
#define PROC_H_

#include <sys/queue.h>

struct process {
	int (*run)(struct process *proc);
	STAILQ_ENTRY(process) proclist;
};

struct process_queue {
	STAILQ_HEAD(process_head, process) head;
	char name[20];
};

struct process_queue *proc_queue_new(const char *id);
void proc_queue_free(struct process_queue *q);
int proc_entry_init(struct process *proc);
int proc_add(struct process_queue *q, struct process *proc);
struct process *proc_next(struct process_queue *q);
struct process *proc_run(struct process_queue *q, int *result_out);
void proc_set_run(struct process *proc, int (*run)(struct process *));
#endif
