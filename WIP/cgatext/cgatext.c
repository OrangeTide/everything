/* cgatext.c : CGA text mode retro graphics - public domain. */
#include "cgatext.h"
#include <stdlib.h>
#include <string.h>

static cgatext_cell *screen;
static int screen_width, screen_height;

/* if region is NULL, return entire screen. else return the original region. */
static inline const cgatext_region *screen_region(const cgatext_region *region)
{
	static cgatext_region tmp;

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
clip_region(const cgatext_region *region, int *x0, int *y0, int *x1, int *y1)
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
cgatext_init(int width, int height)
{
	screen_width = width;
	screen_height = height;

	screen = calloc(screen_width * sizeof(*screen), screen_height);
	if (!screen)
		return -1;

	return cgatext_driver_init();
}

void
cgatext_done(void)
{
	cgatext_driver_done();

	if (screen) {
		free(screen);
		screen_width = screen_height = 0;
		screen = 0;
	}
}

cgatext_cell *cgatext_screen_info(int *width, int *height)
{
	if (width)
		*width = screen_width;
	if (height)
		*height = screen_height;

	return screen;
}

void
cgatext_clear(cgatext_cell ch, const cgatext_region *region)
{
	int x0, y0, x1, y1, x, y;

	region = screen_region(region);

	if (clip_region(region, &x0, &y0, &x1, &y1))
		return;

	for (y = y0; y <= y1; y++) {
		cgatext_cell *p = &screen[x0 + y * screen_width];

		for (x = x0; x <= x1; x++, p++)
			*p = ch;
	}
}

void
cgatext_copyrect(int dstx, int dsty, const cgatext_region *src)
{
	int sx0, sy0, sx1, sy1, sy, dx0, dy0, dx1, dy1, dy;
	cgatext_region dst;
	cgatext_cell *dstp, *srcp;
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
cgatext_vscroll(cgatext_cell fillch, const cgatext_region *region, int scroll)
{
	int dstx, dsty;
	cgatext_region src, clr;

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

	cgatext_copyrect(dstx, dsty, &src);
	cgatext_clear(fillch, &clr);
}

void
cgatext_hscroll(cgatext_cell fillch, const cgatext_region *region, int scroll)
{
	int dstx, dsty;
	cgatext_region src, clr;

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

	cgatext_copyrect(dstx, dsty, &src);
	cgatext_clear(fillch, &clr);
}

void
cgatext_set(cgatext_cell ch, int x, int y)
{
	if (x >= 0 && x < screen_width && y >= 0 && y < screen_height)
		screen[x + y * screen_width] = ch;
}
