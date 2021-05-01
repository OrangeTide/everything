#include "proc.h"

#include <stdio.h>
#include <stdlib.h>

struct process_queue *run_queue;

int
scheduler_init(void)
{
	run_queue = proc_queue_new("RUN");
	return 0;
}

void
scheduler_done(void)
{
	proc_queue_free(run_queue);
}

int
main()
{
	scheduler_init();
	scheduler_done();

	return 0;
}
