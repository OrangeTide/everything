#include <stdio.h>
#include <stdbool.h>

#include "interp.h"

int
main(int argc, char *argv[])
{
	struct interp *in = interp_new();
	if (!in) {
		fprintf(stderr, "error: could not start interpreter.\n");
		return 1;
	}

	if (interp_set_input(in, "<STDIN>", stdin, false) != INTERP_OK) {
		fprintf(stderr, "error: could not set input stream.\n");
		return 1;
	}

	interp_go(in);

	interp_free(in);

	return 0;
}
