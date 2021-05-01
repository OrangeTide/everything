#ifndef DISPLAY_H_
#define DISPLAY_H_
int display_init(void);
void display_done(void);
void display_update(void);
void display_refresh(void);
void display_puts(unsigned short x, unsigned short y, const char *s);
#endif
