#include <locale.h>
#include "screen.h"

static int width, height;

int
main()
{
	setlocale(LC_ALL, ""); /* required */

	if (screen_init(&width, &height))
		return 1;

	screen_done();

	return 0;
}
