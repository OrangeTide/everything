#include "gamepad.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#if defined(__linux__)
#include "gamepad-linux.c"
#elif defined(_WIN32)
#include "gamepad-windows.c"
#else
#error Unsupported platform
#endif

struct gamepad_state gamepad_state[GAMEPAD_MAX];
struct gamepad_info gamepad_info[GAMEPAD_MAX];

/* looks at the state of a single button. */
static inline bool
gamepad_button(int id, int button)
{
	unsigned i, n;

	if (!gamepad_exists(id))
		return false;
	i = button / (sizeof(unsigned) * 8);
	n = button % (sizeof(unsigned) * 8);
	return gamepad_state[id].button[i] & (1UL << n) ? true : false;
}

/* query the state of an axis. */
static inline float
gamepad_axis(int id, int axis)
{
	if (!gamepad_exists(id))
		return NAN;
	return gamepad_state[id].axis[axis];
}

//////// Common

bool
gamepad_init(void)
{
	unsigned i;
	unsigned pads = 0;

	for (i = 0; i < GAMEPAD_MAX; i++)
		if (gamepad_open_one(i) == 0)
			pads++;

	if (!pads)
		return false; /* failure - unable to open any devices */

	return true;
}

void
gamepad_cleanup(void)
{
	gamepad_close_one(0);
}

/* useful for debugging information */
void
gamepad_dump(void)
{
	unsigned i, j;

	for (i = 0; i < GAMEPAD_MAX; i++) {
		if (!gamepad_exists(i))
			continue;
		fprintf(stderr, "pad%u: buttons=%08X%08X\n", i,
				gamepad_state[i].button[0],
				gamepad_state[i].button[1]);
		for (j = 0; j < gamepad_info[i].num_axis; j++ ) {
			fprintf(stderr, "        Axis #%u : %g\n", j, gamepad_axis(i, j));
		}
	}
}

///////// TEST PROGRAM
int
main()
{
	gamepad_init();

	fprintf(stderr, "buttons=%08X%08X\n",
			gamepad_state[0].button[0], gamepad_state[0].button[1]);

	puts("----- press first button on first gamepad to exit -----");
	while (1) {

		if (gamepad_wait(3000)) {
			fprintf(stderr, "EVENTS!\n");

			gamepad_dump();

			/* press first button to exit */
			if (gamepad_button(0, 0))
				break;
		} else {
			fprintf(stderr, "NO EVENTS!\n");
		}
		fprintf(stderr, "TICK\n");
	}

	gamepad_cleanup();
}

// TODO: calibration - do something with JSIOCGCORR
// TODO: implement a dead zone
