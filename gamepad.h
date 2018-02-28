#ifndef GAMEPAD_H
#define GAMEPAD_H
#include <stdbool.h>

#define GAMEPAD_MAX 4
#define GAMEPAD_BUTTON_MAX 64
#define GAMEPAD_AXIS_MAX 8

struct gamepad_state {
	/** dynamic state **/
	unsigned button[(GAMEPAD_BUTTON_MAX + (sizeof(unsigned) * 8 - 1))
		/ (sizeof(unsigned) * 8)]; /* bitmap of buttons */
	float axis[GAMEPAD_AXIS_MAX];

};

struct gamepad_info {
	/** static infomation **/
	unsigned char num_axis;
	unsigned char num_button;
	char name[60];
};

extern struct gamepad_state gamepad_state[GAMEPAD_MAX];
extern struct gamepad_info gamepad_info[GAMEPAD_MAX];

bool gamepad_init(void);
void gamepad_cleanup(void);
bool gamepad_poll(void);
bool gamepad_wait(int msec);
void gamepad_dump(void);

#endif /* GAMEPAD_H */
