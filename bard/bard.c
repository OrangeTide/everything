#include "display.h"
#include "report.h"
#include "poll.h"

#include <stdlib.h>

#define ERR (-1)
#define OK (0)

static void
start(void)
{
	display_refresh();
	while (1) {
		display_poll(1500);
		display_update();
		// TODO: process input
	}
}

static int
process_args(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	// TODO: process command-line arguments

	return OK;
}

int
main(int argc, char **argv)
{
	if (process_args(argc, argv)) {
		report_error("bad argument");
		return EXIT_FAILURE;
	}

	if (display_init()) {
		report_error("screen init failure");
		return EXIT_FAILURE;
	}

	start();

	atexit(display_done);

	return 0;
}
