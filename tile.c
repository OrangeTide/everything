/* tile.c : screen-like tile datastructure and operations - public domain. */
#include <stdlib.h>
#include <string.h>

#include "tile.h"

static tile_cell *screen;
static int screen_width, screen_height;

/* if region is NULL, return entire screen. else return the original region. */
static inline const tile_region *screen_region(const tile_region *region)
{
	static tile_region tmp;
	
	if (region)
		return region;
	
	tmp.x = 0;
	tmp.y = 0;
	tmp.w = screen_width;
	tmp.h = screen_height;
	return &tmp;	
}
	
/* returns 0 on success, 1 if the region is empty or invalid */
static int
clip_region(const tile_region *region, int *x0, int *y0, int *x1, int *y1)
{
	if (!region || !x0 || !y0 || !x1 || !y1)
		return 1;

	*x0 = region->x;
	*y0 = region->y;
	*x1 = region->x + region->w - 1;
	*y1 = region->y + region->h - 1;

	/* clip to screen space */
	if (*x0 < 0)
		*x0 = 0;
	if (*y0 < 0)
		*y0 = 0;
	if (*x1 >= screen_width)
		*x1 = screen_width - 1;
	if (*y1 >= screen_height)
		*y1 = screen_height - 1;

	return (*x0 > *x1 || *y0 > *y1);
}

int
tile_screen_init(int width, int height)
{
	screen_width = width;
	screen_height = height;

	screen = calloc(screen_width * sizeof(*screen), screen_height);
	if (!screen)
		return -1;

	return 0;
}

void
tile_screen_free(void)
{
	if (screen) {
		free(screen);
		screen_width = screen_height = 0;
		screen = 0;
	}
}

const
tile_cell *tile_screen_info(int *width, int *height)
{
	if (width)
		*width = screen_width;
	if (height)
		*height = screen_height;
	return screen;
}

void
tile_clear(tile_cell ch, const tile_region *region)
{
	int x0, y0, x1, y1, x, y;

	region = screen_region(region);

	if (clip_region(region, &x0, &y0, &x1, &y1))
		return;

	for (y = y0; y <= y1; y++) {
		tile_cell *p = &screen[x0 + y * screen_width];

		for (x = x0; x <= x1; x++, p++)
			*p = ch;
	}
}

void
tile_copyrect(int dstx, int dsty, const tile_region *src)
{
	int sx0, sy0, sx1, sy1, sy, dx0, dy0, dx1, dy1, dy;
	tile_region dst;
	tile_cell *dstp, *srcp;
	int len;

	src = screen_region(src);
	if (clip_region(src, &sx0, &sy0, &sx1, &sy1))
		return;

	dst.x = dstx;
	dst.y = dsty;
	dst.w = sx1 - sx0;
	dst.h = sy1 - sy0;
	if (clip_region(&dst, &dx0, &dy0, &dx1, &dy1))
		return;

	len = dx1 - dx0;
	if (dy0 <= sy0) {
		for (dy = dy0, sy = sy0; dy <= dy1; dy++, sy++) {
			dstp = &screen[dx0 + dy * screen_width];
			srcp = &screen[sx0 + sy * screen_width];
			memmove(dstp, srcp, len * sizeof(*dstp));
		}
	} else {
		for (dy = dy1, sy = sy1; dy >= dy0; dy--, sy--) {
			dstp = &screen[dx0 + dy * screen_width];
			srcp = &screen[sx0 + sy * screen_width];
			memmove(dstp, srcp, len * sizeof(*dstp));
		}
	}
}

void
tile_vscroll(tile_cell fillch, const tile_region *region, int scroll)
{
	int dstx, dsty;
	tile_region src, clr;

	region = screen_region(region);

	if (!scroll) {
		return;
	} else if (scroll < 0) {
		/* scroll up (negative) */
		dstx = region->x;
		dsty = region->y;
		src.x = region->x;
		src.y = region->y - scroll;
		src.w = region->w;
		src.h = region->h + scroll;
		clr.x = src.x;
		clr.y = src.y + src.h;
		clr.w = src.w;
		clr.h = -scroll;
	} else {
		/* scroll down (positive) */
		dstx = region->x;
		dsty = region->y + scroll;
		src.x = region->x;
		src.y = region->y;
		src.w = region->w;
		src.h = region->h - scroll;
		clr.x = src.x;
		clr.y = src.y;
		clr.w = src.w;
		clr.h = scroll;
	}

	tile_copyrect(dstx, dsty, &src);
	tile_clear(fillch, &clr);
}

void
tile_hscroll(tile_cell fillch, const tile_region *region, int scroll)
{
	int dstx, dsty;
	tile_region src, clr;

	region = screen_region(region);

	if (!scroll) {
		return;
	} else if (scroll < 0) {
		/* scroll left (negative) */
		dstx = region->x;
		dsty = region->y;
		src.x = region->x - scroll;;
		src.y = region->y;
		src.w = region->w + scroll;
		src.h = region->h;
		clr.x = src.x + src.w;
		clr.y = src.y;
		clr.w = -scroll;
		clr.h = src.h;
	} else {
		/* scroll right (positive) */
		dstx = region->x + scroll;
		dsty = region->y;
		src.x = region->x;
		src.y = region->y;
		src.w = region->w - scroll;
		src.h = region->h;
		clr.x = src.x;
		clr.y = src.y;
		clr.w = scroll;
		clr.h = src.h;
	}

	tile_copyrect(dstx, dsty, &src);
	tile_clear(fillch, &clr);
}

void
tile_set(tile_cell ch, int x, int y)
{
	if (x >= 0 && x < screen_width && y >= 0 && y < screen_height)
		screen[x + y * screen_width] = ch;
}

void
tile_set8x16(tile_cell base, unsigned char ch, int x, int y)
{
		base.id += (TILES_PER_ROW * 2 * (ch / TILES_PER_ROW)) + (ch % TILES_PER_ROW);
		tile_set(base, x, y);
		base.id += TILES_PER_ROW;
		tile_set(base, x, y + 1);
}

/*
 * base : selects color and the first character of the font
 * x,y : starting position relative to clip. updated on return
 */
void
tile_print8x8(tile_cell base, const tile_region *clip, int *x, int *y, const char *s)
{
	unsigned char t;
	int rx, ry;
	tile_cell ch = base;
	
	clip = screen_region(clip);

	rx = clip->x;
	ry = clip->y;
	if (x)
		rx += *x;
	if (y)
		ry += *y;

	while ((t = *s++)) {
		ch.id = base.id + t;

		if (rx > clip->x + clip->w) {
			rx = clip->x; /* wrap to next line */
			ry++;
		}

		if (ry > clip->y + clip->h)
			return; /* outside of clip - not visible */

		tile_set(ch, rx, ry);
		rx++;
	}

	if (x)
		*x = rx - clip->x;
	if (y)
		*y = ry - clip->y;
}

void
tile_print8x16(tile_cell base, const tile_region *clip, int *x, int *y, const char *s)
{
	int rx, ry;

	clip = screen_region(clip);

	rx = clip->x;
	ry = clip->y;
	if (x)
		rx += *x;
	if (y)
		ry += *y * 2;

	while (*s) {
		unsigned char t = *s++;

		if (rx > clip->x + clip->w) {
			rx = clip->x; /* wrap to next line */
			ry += 2;
		}

		if (ry > clip->y + clip->h)
			return; /* outside of clip - not visible */

		tile_set8x16(base, t, rx, ry);
		rx++;
	}

	if (x)
		*x = rx - clip->x;
	if (y)
		*y = (ry - clip->y) / 2;
}

void
tile_print16x16(tile_cell base, const tile_region *clip, int *x, int *y, const char *s)
{
	int rx, ry;
	tile_cell ch = base;
	unsigned char t;

	clip = screen_region(clip);

	rx = clip->x;
	ry = clip->y;
	if (x)
		rx += *x * 2;
	if (y)
		ry += *y * 2;

	while ((t = *s++)) {
		ch.id = base.id +
			(TILES_PER_ROW * 2 * (t / (TILES_PER_ROW / 2))) +
			(2 * (t % (TILES_PER_ROW / 2)));

		if (rx > clip->x + clip->w) {
			rx = clip->x; /* wrap to next line */
			ry += 2;
		}

		if (ry > clip->y + clip->h)
			return; /* outside of clip - not visible */

		tile_set(ch, rx, ry);
		ch.id++;
		tile_set(ch, rx + 1, ry);

		ch.id += TILES_PER_ROW - 1;
		tile_set(ch, rx, ry + 1);

		ch.id++;
		tile_set(ch, rx + 1, ry + 1);

		rx += 2;
	}

	if (x)
		*x = (rx - clip->x) / 2;
	if (y)
		*y = (ry - clip->y) / 2;
}