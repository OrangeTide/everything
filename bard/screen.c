/* screne.c : screen buffer */
#include "screen.h"

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#define ERR (-1)
#define OK (0)

struct screen {
	struct screen_cell fill;
	unsigned short w, h;
	struct screen_cell *cells;
};

static inline size_t
calc_rowbytes(unsigned short w)
{
	return sizeof(struct screen_cell) * w;
}

static int
copy_area(struct screen_cell *dst, unsigned short dst_w, unsigned short dst_h, const struct screen_cell *src, unsigned short src_w, unsigned short src_h, const struct screen_cell *fill)
{
	if (!dst || !src || !dst_w || !dst_h || !src_w || !src_h)
		return ERR;

	size_t dst_rowbytes = calc_rowbytes(dst_w);
	size_t src_rowbytes = calc_rowbytes(src_w);
	/* amount to copy for each row - smaller of dst or src */
	size_t copy_bytes = (dst_rowbytes < src_rowbytes) ? dst_rowbytes : src_rowbytes;

	size_t fill_bytes;
	if (!fill || dst_rowbytes <= src_rowbytes) { /* no fill cell */
		fill_bytes = 0;
	} else if (dst_rowbytes > src_rowbytes) { /* destination will need to use fill cell */
		fill_bytes = dst_rowbytes - src_rowbytes;
	}

	/* choose smaller area of dst versus src */
	unsigned h = (dst_h < src_h) ? dst_h : src_h;
	unsigned y;
	for (y = 0; y < h; y++) {
		memcpy(dst, src, copy_bytes);

		/* handle the fill cell */
		if (fill_bytes) {
			struct screen_cell *cur = (struct screen_cell*)((char*)dst + copy_bytes);
			size_t i = 0;

			/* copy fill cell into every position it can fit */
			for (i = 0; i + sizeof(struct screen_cell) <= fill_bytes; i += sizeof(struct screen_cell)) {
				*cur = *fill;
				cur++;
			}

			/* set any padding to 0 */
			if (i < fill_bytes) {
				memset(cur, 0, fill_bytes - i);
			}
		}

		/* move to next row */
		dst = (struct screen_cell*)((char*)dst + dst_rowbytes);
		src = (const struct screen_cell*)((const char*)src + src_rowbytes);
	}

	/* do we need to fill rows in a dst taller than src? */
	if (fill && y < dst_h) {
		/* slowly create one blank row with the fill character */
		struct screen_cell *blank_row = dst;
		unsigned short i;
		for (i = 0; i < dst_w; i++) {
			dst[i] = *fill;
		}

		/* set any padding to 0 */
		if ((&dst[i] - dst) < (ptrdiff_t)dst_rowbytes) {
			memset(&dst[i], 0, dst_rowbytes - (&dst[i] - dst));
		}

		/* move to next row */
		dst = (struct screen_cell*)((char*)dst + dst_rowbytes);
		y++;

		/* use initial blank row to fill remaining rows (if any) */
		for (; y < dst_h; y++) {
			memcpy(dst, blank_row, dst_rowbytes);
		}
	}

	return OK;
}

struct screen *
screen_new(unsigned short w, unsigned short h)
{
	struct screen *scr = calloc(1, sizeof(*scr));
	if (!scr)
		return NULL;

	scr->w = w;
	scr->h = h;

	strncpy((char*)scr->fill.ch, " ", SCREEN_CELL_MAX_CHARS);
	scr->fill.attr.attr = 0;
	scr->fill.attr.fg = 7;
	scr->fill.attr.bg = 0;

	scr->cells = calloc(h, calc_rowbytes(w));
	if (!scr->cells) {
		free(scr);
		return NULL;
	}

	return scr;
}

void
screen_free(struct screen *scr)
{
	if (scr) {
		free(scr->cells);
		scr->cells = NULL;
		scr->w = scr->h = 0;
		free(scr);
	}
}

int
screen_update_size(struct screen *scr, unsigned short w, unsigned short h)
{
	if (!scr)
		return ERR;

	if (w == scr->w && h == scr->h)
		return OK; /* no-op: ignore requests for identical size */

	/* use 0x0 in width or height to free the backing structure */
	if (!w || !h) {
		if (scr->cells) {
			free(scr->cells);
			scr->cells = NULL;
		}
		scr->w = scr->h = 0;
		return OK;
	}

	struct screen_cell *newcells = calloc(h, calc_rowbytes(w));
	if (!newcells)
		return ERR;

	copy_area(newcells, w, h, scr->cells, scr->w, scr->h, &scr->fill);

	free(scr->cells);
	scr->cells = newcells;
	scr->w = w;
	scr->h = w;

	return OK;
}
