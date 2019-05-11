/* cgatext.h : CGA text mode retro graphics - public domain. */
#ifndef CGATEXT_H
#define CGATEXT_H
/* TODO:
 * remove cgatext_screen_info() and direct writing to screen buffer
 * rename cgatext_set() to cgatext_putc()
 * change drawing model to make it easier to implement blinking cursor
 * rename init/done to open/close.
 * add timeout for process_events()
 * optional - rename to cgatext to b800
 */

#define CHAR_PART(cell) ((cell).ch)
#define FG_PART(cell) ((cell).fg)
#define BG_PART(cell) ((cell).bg)

typedef struct { unsigned short ch; unsigned char fg, bg; } cgatext_cell;

typedef struct { int x, y, w, h; } cgatext_region;

typedef enum cgatext_cursor_style {
	CGATEXT_CURSOR_INVISIBLE,
	CGATEXT_CURSOR_BAR,
	CGATEXT_CURSOR_BLOCK,
} cgatext_cursor_style_t;

int cgatext_init(int width, int height);
void cgatext_done(void);
cgatext_cell *cgatext_screen_info(int *width, int *height);
void cgatext_clear(cgatext_cell ch, const cgatext_region *region);
void cgatext_copyrect(int dstx, int dsty, const cgatext_region *src);
void cgatext_hscroll(cgatext_cell fillch, const cgatext_region *region, int scroll);
void cgatext_vscroll(cgatext_cell fillch, const cgatext_region *region, int scroll);
void cgatext_set(cgatext_cell ch, int x, int y);
int cgatext_process_events(int timeout_msec); /* return 0 = OK, negative = quit or error */
void cgatext_refresh(void);
int cgatext_driver_init(void); /* internal use - do not call in application */
void cgatext_driver_done(void); /* internal use - do not call in application */
void cgatext_cursor_style(cgatext_cursor_style_t style);
void cgatext_cursor(int x, int y);
#endif
