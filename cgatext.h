/* cgatext.h : CGA text mode retro graphics - public domain. */
#ifndef CGATEXT_H
#define CGATEXT_H

typedef struct { unsigned short ch; unsigned char fg, bg; } cgatext_cell;

typedef struct { int x, y, w, h; } cgatext_region;

int cgatext_init(int width, int height);
void cgatext_done(void);
cgatext_cell *cgatext_screen_info(int *width, int *height);
void cgatext_clear(cgatext_cell ch, const cgatext_region *region);
void cgatext_copyrect(int dstx, int dsty, const cgatext_region *src);
void cgatext_hscroll(cgatext_cell fillch, const cgatext_region *region, int scroll);
void cgatext_vscroll(cgatext_cell fillch, const cgatext_region *region, int scroll);
void cgatext_set(cgatext_cell ch, int x, int y);
int cgatext_process_events(void); /* 0 = OK, negative = quit or error */
void cgatext_refresh(void);
int cgatext_driver_init(void); /* internal use - do not call in application */
void cgatext_driver_done(void); /* internal use - do not call in application */
#endif
