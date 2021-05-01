#include "report.h"

#include <stdio.h>

void
report_error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
}
