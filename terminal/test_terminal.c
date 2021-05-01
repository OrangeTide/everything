/* vim: set ai tw=75: */
#include "terminal.h"
int
main(/* int argc, char **argv */)
{
	terminal_open();

	/* try out a bunch of colors and things */
	unsigned x, y;
	for (x = 0; x < 16; x++)
		terminal_write(GOTOXY(x + 8, 0), CHATTR("#", MKATTR(x, 0)));
	for (y = 0; y < 16; y++)
		for (x = 0; x < 16; x++)
			terminal_write(GOTOXY(x + 16, y + 1), CHATTR("*", MKATTR(0, x + y * 16)));

	terminal_refresh();
	terminal_close();

	return 0;
}
