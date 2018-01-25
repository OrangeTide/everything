#include <stdlib.h>
#include <stdio.h>
#include "notifier.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"

/* loaded apps ... */
#include "app-calc.c"

void
app_initialize(int width, int height)
{
	// TODO: register some notifiers for each loaded app
}

void
app_terminate(void)
{
}

void
app_paint(struct nk_context *ctx)
{
	// TODO: go through list of notifiers instead of hardcoding
	calculator(ctx);
}
