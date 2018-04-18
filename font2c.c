/* font2c.c : converts a BMP font file to a C array - public domain */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

struct state {
	FILE *inf, *outf;
	const char *inname, *outname;
	unsigned cell_width, cell_height;
	unsigned columns, cells;

	void *bmp_data;
	size_t bmp_len;

	void *pixels;
};

static FILE *
open_in(const char *name)
{
	FILE *inf;

	inf = fopen(name, "rb");
	if (!inf) {
		perror(name);
		return NULL;
	}

	return inf;
}

static FILE *
open_out(const char *name)
{
	FILE *outf;

	outf = fopen(name, "w");
	if (!outf) {
		perror(name);
		return NULL;
	}

	return outf;
}

static void
cleanup_state(struct state *st)
{
	if (!st)
		return;
	if (st->bmp_data)
		free(st->bmp_data);
	st->bmp_data = NULL;
	if (st->outf)
		fclose(st->outf);
	st->outf = NULL;
	if (st->inf)
		fclose(st->inf);
	st->inf = NULL;
}

static int
initialize_state(struct state *st)
{
	memset(st, 0, sizeof(*st));

	st->cell_width = 16; // TODO: make configurable
	st->cell_height = 16; // TODO: make configurable
//	st->bpp = 1;
	st->columns = 32; // TODO: make configurable
	st->cells = 256; // TODO: make configurable

	return 0;
}

static int
parse_args(int argc, char **argv, struct state *st)
{
	char **cur, n;

	for (cur = argv + 1, n = 0; cur < argv + argc; cur++) {
		const char *arg = *cur;
		switch (n) {
		case 0: /* input file */
			n++;
			if (arg[0] == '-' && !arg[1]) {
				st->inname = "<STDIN>";
				st->inf = stdin;
			} else {
				st->inname = arg;
				st->inf = open_in(st->inname);
			}
			if (!st->inf)
				goto failure;
			break;
		case 1: /* output file */
			n++;
			if (arg[0] == '-' && !arg[1]) {
				st->outname = "<STDOUT>";
				st->outf = stdout;
			} else {
				st->outname = arg;
				st->outf = open_out(st->outname);
			}
			if (!st->outf)
				goto failure;
			break;
		default:
			fprintf(stderr, "usage: %s <in-file.bmp> [<out-file.c>]\n", argv[0]);
			return 1;
		}
	}

	if (n == 1) {
		st->outname = "<STDOUT>";
		st->outf = stdout;
	}

	if (n > 0)
		return 0; /* success */
failure:
	return -1; /* failure */
}

/* minimally decodes BMP and provides a pointer to its data */
static void *
bmp_pixels(const void *bmpdata, unsigned *out_width, unsigned *out_height)
{
	const void *bits;
	unsigned width, height;

	// TODO: make this code portable to big-endian systems

	bits = (const char*)bmpdata + *(uint32_t*)((char*)bmpdata + 10);

	width = *(uint32_t*)((char*)bmpdata + 18),
	height = *(uint32_t*)((char*)bmpdata + 22);

	if (out_width)
		*out_width = width;
	if (out_height)
		*out_height = height;

	return (void*)bits;
}

/* allocate a file into a buffer */
static void *
load_file(FILE *f, const char *name, size_t *len)
{
	void *buf;
	size_t _len;

	fseek(f, 0, SEEK_END);
	_len = ftell(f);
	fseek(f, 0, SEEK_SET);
	buf = malloc(_len);
	if (!buf) {
		perror(name);
		return NULL;
	}
	fread(buf, 1, _len, f);
	if (ferror(f)) {
		perror(name);
		free(buf);
		return NULL;
	}

	if (len)
		*len = _len;
	return buf;
}

/* blits between 1 bit buffers. must be aligned to byte boundries */
static int
copy_bits_1bpp(char *dst, unsigned dst_pitch, const char *src, unsigned src_pitch,
	unsigned width, unsigned height)
{
	char *dp;
	const char *sp;
	unsigned x, y;

	for (dp = dst, sp = src, y = 0; y < height; dp += dst_pitch, sp += src_pitch, y++) {
		for (x = 0; x < width; x += 8) {
			*dp = *sp;
		}
	}

	return 0;
}

static int
bmp_load(FILE *f, const char *name, struct state *st)
{
	char *buf;
	size_t len;
	unsigned font_rows, font_pitch;
	const char *bmppix;
	unsigned bmp_width, bmp_height, bmp_pitch;
	unsigned ch;

	/* load BMP file */
	buf = load_file(f, name, &len);
	if (!buf)
		return -1; /* error */
	bmppix = bmp_pixels(buf, &bmp_width, &bmp_height);
	if  (!bmppix) {
		free(buf);
		return -1;
	}
	bmp_pitch = (bmp_width + 7) / 8; /* 1 bpp */ // TODO: support other formats
	st->bmp_data = buf;
	st->bmp_len = len;

	// TODO: set st->columns and st->cells based on bmp dimensions

	/* create font buffer - it is 1 character wide, and 256 characters tall (or more) */
	font_pitch = (st->cell_width + 7) / 8; /* 1 bpp - round up to whole bytes */
	font_rows = st->cell_height * st->cells;
	st->pixels = calloc(font_rows, font_pitch);
	if (!st->pixels) {
		perror(name);
		return -1;
	}

	/* process BMP file data into st->pixels for font */
	for (ch = 0; ch < st->cells; ch++) {
		char *dst = (char*)st->pixels + ch * font_pitch * st->cell_height;
		const char *src = bmppix + (ch / st->columns) * bmp_pitch + ((ch % st->columns) * st->cell_width / 8);
		// NOTE: the way src is calculated means fonts must start on a byte boundry.
		copy_bits_1bpp(dst, font_pitch, src, bmp_pitch, st->cell_width, st->cell_height);
	}

	return 0;
}

/* create a safe name from the original filename */
static int
make_safename(char *safename, size_t max, const char *filename)
{
	const char *b;
	unsigned i;

	/* guess at the base name */
	b = strrchr(filename, '/');
	if (!b)
		b = strrchr(filename, '\\');
	if (!b)
		b = filename - 1;
	b++;

	i = 0;

	/* if it starts with a digit, prefix an 'n' */
	if (isdigit(*b))
		safename[i++] = 'n';

	for (; i < max - 2 && *b && *b != '.'; b++) {
		if (isspace(*b))
			safename[i++] = '_';
		else if (!isalnum(*b))
			safename[i++] = *b;
		/* ignore other characters */
	}
	safename[i] = 0; /* null terminator */

	return i ? 0 : -1;
}

static int
font_save(FILE *f, const char *name, struct state *st)
{
	unsigned ch, y, x;
	const unsigned char *p;
	unsigned font_pitch;
	char safename[32];

	if (!name)
		return -1;

	if (make_safename(safename, sizeof(safename), name))
		strcpy(safename, "font"); /* fall back to a default on error */

	font_pitch = (st->cell_width + 7) / 8;

	/* output C array */
	fprintf(f, "const %s_data[] = {\n", safename);
//	for (ch = 0; ch < st->cells; ch++) {
	for (ch = 'A'; ch <= 'Z'; ch++) { // test code for A-Z
		if (isprint(ch))
			fprintf(f, "\t/* 0x%02x '%c' */\n", ch, ch);
		else
			fprintf(f, "\t/* 0x%02x */\n", ch);

		p = (const unsigned char*)st->pixels + ch * font_pitch * st->cell_height;

		for (y = 0; y < st->cell_height; y++, p += font_pitch) {
			fprintf(f, "\t");
			for (x = 0; x < st->cell_width; x += 8) {
				fprintf(f, "0x%02x,%s", p[x / 8], (x + 8) < st->cell_width ? " " : "");
			}
			fprintf(f, "\n");
		}
	}
	fprintf(f, "};\n");

	return 0;
}

int
main(int argc, char **argv)
{
	struct state state;

	if (initialize_state(&state)) {
		printf("ERROR in initialization\n");
		return 1;
	}

	if (parse_args(argc, argv, &state)) {
		printf("ERROR parsing arguments\n");
		return 1; // TODO: cleanup_state()
	}

	if (bmp_load(state.inf, state.inname, &state)) {
		printf("ERROR loading BMP!\n");
		return 1; // TODO: cleanup_state()
	}

	if (font_save(state.outf, state.outname, &state)) {
		printf("ERROR saving font!\n");
		return 1; // TODO: cleanup_state()
	}

	cleanup_state(&state);
	return 0;
}