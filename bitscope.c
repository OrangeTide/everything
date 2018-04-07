/* bitscope.c : retrocomputer - public domain. */
#include <stdio.h>

#include "bitscope.h"

#if defined(WIN32)
#include <windows.h>
#endif

#ifdef NDEBUG
#  define DBG_LOG(...) /* disabled */
#else
#  define DBG_LOG(f, ...) fprintf(stderr, f "\n", ## __VA_ARGS__)
#endif

void
bitscope_paint(unsigned char *pixels, unsigned width, unsigned height, unsigned pitch)
{
	unsigned char *p;
	unsigned x, y;

	for (y = 0, p = pixels; y < height; y++, p = (void*)((char*)p + pitch))
		for (x = 0; x < width; x++)
			p[x] = x ^ y;

}

/* reads exactly 2 hex digits */
int
decode_hex_byte(const char buf[2])
{
	char ch;
	const char *s;
	unsigned n = 0;

	if (!buf[0])
		return -1;

	for (s = buf; s < (buf + 2) && (ch = *s++); ) {
		n *= 16;
		switch (ch) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			n += ch - '0';
			break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			n += ch - 'a' + 10;
			break;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			n += ch - 'A' + 10;
			break;
		default:
			return -1; /* parse error */
		}
	}
	return n;
}

/* reads exactly 4 hex digits */
int
decode_hex_word(const char buf[4])
{
	int a, b;

	a = decode_hex_byte(buf);
	if (a < 0)
		return -1;
	b = decode_hex_byte(buf + 2);
	if (b < 0)
		return -1;

	return (b << 8) | a;
}

/* I8HEX files use only record types 00 and 01 (16 bit addresses) */
int
load_hex(const char *filename, int (*process)(unsigned address, unsigned count, unsigned char *data))
{
	FILE *f;
	char line[80], *s;
	int i, count, addr, rec, cksum;
	unsigned char out[256], total;
	int end_of_file;
	int line_num;

	f = fopen(filename, "r");
	if (!f) {
		perror(filename);
		return -1;
	}

	end_of_file = 0;
	total = 0;
	line_num = 0;
	while (fgets(line, sizeof(line), f)) {
		s = line;
		line_num++;

		while (*s == ' ' || *s == '\t')
			s++;

		/* requires the start code : */
		if (*s != ':') {
			fprintf(stderr, "%s:%d:parse error:missing start code\n", filename, line_num);
			goto failure;
		}
		s++;

		/* two hex digit byte count */
		count = decode_hex_byte(s);
		if (count < 0) {
			fprintf(stderr, "%s:%d:parse error:missing byte count\n", filename, line_num);
			goto failure;
		}
		s += 2;
		total += count;

		if (count > (int)sizeof(out)) {
			fprintf(stderr, "%s:%d:parse error:byte count exceeds maximum supported\n", filename, line_num);
			goto failure; 	/* error: limited to the size of out[] */
		}

		/* four hex digit address */
		addr = decode_hex_word(s);
		if (addr < 0) {
			fprintf(stderr, "%s:%d:parse error:missing address information\n", filename, line_num);
			goto failure;
		}
		s += 4;
		total += addr & 255;
		total += (addr >> 8) & 255;

		/* two hex digit record type */
		rec = decode_hex_byte(s);
		if (rec < 0) {
			fprintf(stderr, "%s:%d:parse error:missing record type\n", filename, line_num);
			goto failure;
		}
		s += 2;
		total += rec;

		if (rec == 0) { /* data record */
			/* decode count bytes of data (2 hex digits per byte) */
			for (i = 0; i < count; i++) {
				int data;

#ifndef NDEBUG /* paranoid check */
				if (s + 2 >= line + sizeof(line)) {
					fprintf(stderr, "%s:%d:buffer overflow\n", filename, line_num);
					goto failure;
				}
#endif
				data = decode_hex_byte(s);
				if (data < 0) {
					fprintf(stderr, "%s:%d:parse error:data parse failure\n", filename, line_num);
					goto failure;
				}
				s += 2;
				total  += data;

				out[i] = data;
			}
		} else if (rec == 1) {
			end_of_file = 1;
		} else {
			fprintf(stderr, "%s:%d:parse error:unknown record type $%02X\n", filename, line_num, rec);
			goto failure;
		}

		/* two hex digit checksum */
		cksum = decode_hex_byte(s);
		if (cksum < 0) {
			fprintf(stderr, "%s:%d:parse error:checksum parse failure\n", filename, line_num);
			goto failure;
		}
		s += 2;
		total += cksum;

		if (total) { /* cksum should have made the total $00 */
			fprintf(stderr, "%s:%d:parse error:data checksum mismatch ($%02X != $%02X)\n", filename, line_num, cksum, total);
			goto failure; /* checksum failed */
		}

		if (*s != 0 && *s != '\r' && *s != '\n')
				goto failure; /* some trailing junk was present */

		if (rec == 0 && process)
			process(addr, count, out);
	}
	if (!end_of_file) {
		fprintf(stderr, "%s:%d:truncated file detected (missing EOF record)\n", filename, line_num);
		goto failure;
	}
	fclose(f);
	return 0;

failure:
	fclose(f);

#ifndef NDEBUG
	/* interactive prompts for errors */
	printf("Press enter to proceed\n");
	getchar();
#endif

	return -1;
}

int
bitscope_load(void)
{
	int e;

	e = load_hex("ROM.HEX", NULL);
	fprintf(stderr, "e=%d\n", e);
	if (e)
		return -1;

	// TODO: implement this
	return 0;
}

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
#if !defined(NDEBUG) && defined(WIN32)
	AllocConsole();
	freopen("CONIN$", "r",stdin);
	freopen("CONOUT$", "w",stdout);
	freopen("CONOUT$", "w",stderr);
#endif
	DBG_LOG("Starting up ...");

	if (bitscope_init())
		return 1;

	bitscope_loop();

	bitscope_fini();

	return 0;
}