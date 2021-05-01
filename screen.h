#ifndef SCREEN_H
#define SCREEN_H

#define FG_PART(cell) ((cell).fg)
#define BG_PART(cell) ((cell).bg)
#define CHAR_PART(cell) ((cell).ch)

typedef struct screen_cell { unsigned short ch; unsigned char fg, bg; } screen_cell_t;
typedef enum screen_cursor_style { SCREEN_CURSOR_STYLE_INVISIBLE, SCREEN_CURSOR_STYLE_NORMAL, } screen_cursor_style_t;

void screen_putc(int x, int y, unsigned short ch, unsigned char fg, unsigned char bg);
void screen_vgaput(int x, int y, unsigned char ch, unsigned char attr);
int screen_puts(int x, int y, unsigned char fg, unsigned char bg, const char *s);
int screen_init(int *width, int *height);
void screen_done(void);
#endif
