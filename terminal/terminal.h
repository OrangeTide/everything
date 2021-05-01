#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stddef.h>

#define CHAR_PART(cell)  ((cell).ch)
// TODO(remove): #define ATTR_PART(cell)  ((cell).fg | ((unsigned short)(cell).bg << 8u))
#define FG_PART(cell)  ((cell).fg)
#define BG_PART(cell)  ((cell).bg)
#define CHATTR(c, attr) ((struct cell){.fg = (attr) & 0xffu, .bg = ((attr) >> 8u) & 0xffu, .ch = (c)})
#define MKATTR(fg, bg) (((fg) & 0xffu) | ((unsigned short)(bg) << 8u))
#define GOTOXY(x, y) ((x) + ((y) * vstride))

#define FGCOLOR_BLACK (0u)
#define FGCOLOR_NAVY (1u)
#define FGCOLOR_FOREST (2u)
#define FGCOLOR_AQUA (3u)
#define FGCOLOR_CRIMSON (4u)
#define FGCOLOR_VIOLET (5u)
#define FGCOLOR_BROWN (6u)
#define FGCOLOR_GRAY (7u)
#define FGCOLOR_GLOOM (8u)
#define FGCOLOR_BLUE (9u)
#define FGCOLOR_GREEN (10u)
#define FGCOLOR_CYAN (11u)
#define FGCOLOR_RED (12u)
#define FGCOLOR_PURPLE (13u)
#define FGCOLOR_YELLOW (14u)
#define FGCOLOR_WHITE (15u)

#define BGCOLOR_BLACK (0x00u)
#define BGCOLOR_NAVY (0x10u)
#define BGCOLOR_FOREST (0x20u)
#define BGCOLOR_AQUA (0x30u)
#define BGCOLOR_CRIMSON (0x40u)
#define BGCOLOR_VIOLET (0x50u)
#define BGCOLOR_BROWN (0x60u)
#define BGCOLOR_GRAY (0x70u)
#define BGCOLOR_GLOOM (0x80u)
#define BGCOLOR_BLUE (0x90u)
#define BGCOLOR_GREEN (0xA0u)
#define BGCOLOR_CYAN (0xB0u)
#define BGCOLOR_RED (0xC0u)
#define BGCOLOR_PURPLE (0xD0u)
#define BGCOLOR_YELLOW (0xE0u)
#define BGCOLOR_WHITE (0xF0u)

/******************************************************************************/

struct cell {
	unsigned char fg, bg;
	unsigned char ch[6];
};

/******************************************************************************/

extern struct cell *screen;
extern size_t screen_length;
extern unsigned short vheight, vwidth;
extern unsigned vstride;

/******************************************************************************/

static inline void
terminal_write(unsigned ofs, struct cell cell)
{
	if (ofs > screen_length)
		return;
	screen[ofs] = cell;
}

/******************************************************************************/

int terminal_open(void);
void terminal_close(void);
void terminal_refresh(void);
#endif
