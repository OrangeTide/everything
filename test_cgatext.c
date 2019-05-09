/* test_cgatext.c : exercises all of the cgatext library - public domain. */
#include "cgatext.h"

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	int width, height;

	if (cgatext_init(80, 25))
		return 1;

	cgatext_cell *screen = cgatext_screen_info(&width, &height);
	if (!screen)
		return 1;

	int frame = 0;
	do {
		/* draw some simple thing */
		int x, y;
		cgatext_cell *row = screen;
		for (y = 0; y < height; y++, row += width) {
			for (x = 0; x < width; x++) {
				row[x].ch = 'A' + (frame % 26);
				row[x].fg = (frame + x) % 16;
				row[x].bg = (frame + y) % 16;
			}
		}

		cgatext_refresh();
		if (cgatext_process_events())
			break;

		// TODO: check for keys

		frame++;
		if (frame >= (26 * 16))
			frame = 0;
	} while (1);

	cgatext_done();

	return 0;
}
