#ifndef SCREEN_H_
#define SCREEN_H_

#define SCREEN_CELL_MAX_CHARS 8

struct screen_attribute {
	unsigned char fg, bg;
	unsigned short attr;
};

struct screen_cell {
	unsigned char ch[SCREEN_CELL_MAX_CHARS];
	struct screen_attribute attr;
};

struct screen;

struct screen *screen_new(unsigned short w, unsigned short h);
void screen_free(struct screen *scr);
int screen_update_size(struct screen *scr, unsigned short w, unsigned short h);

#endif
