/* tile.h : screen-like tile datastructure and operations - public domain. */
#ifndef TILE_H
#define TILE_H

#define TILES_PER_ROW 32

typedef struct { unsigned short id; unsigned fg, bg; } tile_cell;

typedef struct { int x, y, w, h; } tile_region;

int tile_screen_init(int width, int height);
void tile_screen_free(void);
const tile_cell *tile_screen_info(int *width, int *height);
void tile_clear(tile_cell ch, const tile_region *region);
void tile_copyrect(int dstx, int dsty, const tile_region *src);
void tile_hscroll(tile_cell fillch, const tile_region *region, int scroll);
void tile_vscroll(tile_cell fillch, const tile_region *region, int scroll);
void tile_set(tile_cell ch, int x, int y);
void tile_set8x16(tile_cell base, unsigned char ch, int x, int y);
void tile_print8x8(tile_cell color, const tile_region *clip, int *x, int *y, const char *s);
void tile_print8x16(tile_cell color, const tile_region *clip, int *x, int *y, const char *s);
void tile_print16x16(tile_cell color, const tile_region *clip, int *x, int *y, const char *s);
#endif
