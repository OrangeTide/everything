/* jdm_stream.h : stream reading and token parsing - public domain. */
#ifndef JDM_STREAM_H
#define JDM_STREAM_H

/* Usage:
 *
 * #include <stdio.h>
 *
 * #define JDM_STREAM_PARSING_IMPLEMENTATION
 * #include "jdm_stream.h"
 *
 * void main() {
 *     char line[256];
 *     double fl;
 *     long num;
 *     char ident[8];
 *     struct stream_info p;
 *     while (fgets(line, sizeof(line), stdin)) {
 *         stream_initialize(&p, line, 0);
 *         if (stream_read_float(&p, &fl)) {
 *             printf("Started with float %g, ", fl);
 *             if (stream_read_integer(&p, &num))
 *                 printf("followed by integer %ld, ", num);
 * 	        printf("ends with: %s\n", stream_remaining(&p, NULL));
 *         } else if (stream_read_identifier(&p, ident, sizeof(ident))) {
 *             printf("Started with identifier %s, ", ident);
 *             printf("ends with: %s\n", stream_remaining(&p, NULL));
 *         } else {
 *             printf("an error occurred\n");
 *         }
 *     }
 * }
 *
 */

#include <stddef.h>
#include <stdbool.h>

struct stream_info {
	const char *base;
	const char *cur;
	size_t len;
	unsigned lineno;
	int error;
	int unget;
};

/******************************************************************************/
/* core stream input functions */
/******************************************************************************/

int stream_initialize(struct stream_info *p, const char *s, size_t len);
const char *stream_remaining(struct stream_info *p, size_t *remaining);
int stream_rewind(struct stream_info *p);
int stream_peekc(struct stream_info *p);
int stream_getc(struct stream_info *p);
int stream_ungetc(struct stream_info *p, char ch);
void stream_set_error(struct stream_info *p);

/******************************************************************************/
/* stream parsing functions */
/******************************************************************************/
bool stream_discard_spaces(struct stream_info *p);
bool stream_read_integer(struct stream_info *p, long *out);
bool stream_read_float(struct stream_info *p, double *out);
bool stream_read_identifier(struct stream_info *p, char *ident_out, size_t ident_max);

#if defined(JDM_IMPLEMENTATION) || defined(JDM_STREAM_PARSING_IMPLEMENTATION)

/* core implementation */

#include <string.h>
#include <ctype.h>

/* s can be a length terminated block of memory if len is non-zero.
 * else s must be a NUL terminated string.
 */
int
stream_initialize(struct stream_info *p, const char *s, size_t len)
{
	p->base = s;
	p->len = len ? len : strlen(s);

	return stream_rewind(p);
}

int
stream_rewind(struct stream_info *p)
{
	p->cur = p->base;
	p->lineno = 1;
	p->error = 0;
	p->unget = EOF; /* means there is nothing in the unget buffer */

	return 0;
}

/* returns tail of the string or NULL if end-of-string
 * if remaining is not NULL, updates it with number of bytes left.
 */
const char *
stream_remaining(struct stream_info *p, size_t *remaining)
{
	if (!p->cur || p->cur >= p->base + p->len)
		return NULL;

	if (remaining)
		*remaining = p->len - (p->cur - p->base);

	return p->cur;	
}

int
stream_peekc(struct stream_info *p)
{
	int ch;
	
	if (!p || p->error || !p->cur || (p->cur >= p->base + p->len))
		goto endoffile;

	if (p->unget != EOF) {
		fprintf(stderr, "%s():restoring unget\n", __func__);
		return p->unget;
	}


	ch = (unsigned char)*p->cur;

	if (!ch)
		goto endoffile;


	return ch;
endoffile:
	if (p->error)
		fprintf(stderr, "%s():error\n", __func__);
	else
		fprintf(stderr, "%s():end-of-file\n", __func__);
	return EOF;
}

int
stream_getc(struct stream_info *p)
{
	int ch;

	ch = stream_peekc(p);
	if (ch != EOF) {
		if (p->unget == EOF)
			p->cur++;
		else
			p->unget = EOF;
	}
	if (ch == '\n')
		p->lineno++;

	return ch;
}

int
stream_ungetc(struct stream_info *p, char ch)
{
	if (ch != EOF && p->unget != EOF)
		return -1;

	p->unget = (unsigned char)ch;

	return 0;
}

void
stream_set_error(struct stream_info *p)
{
	fprintf(stderr, "%s():error\n", __func__);
	p->error = 1;
}
#endif

#ifdef JDM_STREAM_PARSING_IMPLEMENTATION

/* parsing related routines */

#include <stdlib.h>

/* discard spaces and tabs */
bool
stream_discard_spaces(struct stream_info *p)
{
	int ch;

	ch = stream_peekc(p);
	
	if (ch != ' ' && ch != '\t')
		return false;

	while ((ch = stream_peekc(p)) == ' ' || ch == '\t')
		stream_getc(p);

	return true;
}

/* reads a series of 0-9 digits into a string */
static bool
stream_read_unsigned(struct stream_info *p, char *num_out, size_t num_max, size_t *cnt_out)
{
	size_t cnt;
	int ch;

	ch = stream_peekc(p);

	if (!isdigit(ch))
		return false;

	cnt = 0;
	do {
		stream_getc(p);
		if (cnt < num_max) {
			if (num_out)
				num_out[cnt] = ch;
			cnt++;
		}
		ch = stream_peekc(p);
	} while (isdigit(ch));

	if (cnt_out)
		*cnt_out = cnt;

	if (cnt >= num_max) {
		fprintf(stderr, "%s():buffer overflow\n", __func__);
		stream_set_error(p); /* error: buffer overflow */
		if (num_out && num_max)
			*num_out = 0;
		return false;
	}
	num_out[cnt] = 0;

	return true;
}

/* matched [+|-]?[0-9]+ into a string */
static bool
stream_read_signed(struct stream_info *p, char *num_out, size_t num_max, size_t *cnt_out)
{
	int ch;
	size_t cnt, tmp;

	cnt = 0;

	ch = stream_peekc(p);
	if (ch == '-' || ch == '+') {
		stream_getc(p);
		if (num_out && cnt < num_max)
			num_out[cnt] = ch;
		cnt++;
		ch = stream_peekc(p);
	}

	if (!stream_read_unsigned(p, num_out + cnt, num_max - cnt, &tmp)) {
		stream_ungetc(p, ch);
		return false;
	}

	cnt += tmp;

	if (*cnt_out)
		*cnt_out = cnt;

	return true;
}


/* reads an integer */
bool
stream_read_integer(struct stream_info *p, long *out)
{
	char buf[24];
	size_t cnt;

	stream_discard_spaces(p);

	if (stream_read_signed(p, buf, sizeof(buf), &cnt)) {
		if (out)
			*out = atol(buf);
		return true;
	}
	
	if (out)
		*out = 0;

	return false;
}

bool
stream_read_float(struct stream_info *p, double *out)
{
	int ch;
	char buf[32];
	size_t cnt, tmp;

	/* supports the follow forms
	 * 0
	 * .0
	 * 0.
	 * 0.0
	 * 0e+1
	 * .0e+1
	 * 0.e+1
	 * 0.0e+1
	 *
	 */

	stream_discard_spaces(p);

	cnt = 0;

	ch = stream_peekc(p);

	if (isdigit(ch) || ch == '+' || ch == '-') {
		stream_read_signed(p, buf + cnt, sizeof(buf) - cnt, &tmp);
		cnt += tmp;
		ch = stream_peekc(p);
	} else if (ch != '.') {
		return false; /* not a float */
	}

	if (ch == '.') {
		stream_getc(p);
		if (cnt < sizeof(buf))
			buf[cnt++] = ch;
		if (!stream_read_unsigned(p, buf + cnt, sizeof(buf) - cnt, &tmp))
			goto done;
		cnt += tmp;
		ch = stream_peekc(p);
	}

	if (ch == 'e' || ch == 'E') {
		stream_getc(p);
		if (cnt < sizeof(buf))
			buf[cnt++] = ch;
		ch = stream_peekc(p);

		if (ch == '-' || ch == '+') {
			stream_getc(p);
			if (cnt < sizeof(buf))
				buf[cnt++] = ch;
			ch = stream_peekc(p);
		}

		if (!stream_read_unsigned(p, buf + cnt, sizeof(buf) - cnt, &tmp))
			goto failure;
		cnt += tmp;
	}

done:
	if (cnt >= sizeof(buf))
		goto failure;

	buf[cnt] = 0;

	if (out)
		*out = atof(buf);

	return true;

failure:
	stream_set_error(p); /* error: buffer overflow */
	if (out)
		*out = 0.0;
	return false;
}

/* read C-style identifier */
bool
stream_read_identifier(struct stream_info *p, char *ident_out, size_t ident_max)
{
	size_t cnt;
	int ch;

	ch = stream_peekc(p);

	if (!isalpha(ch) && ch != '_')
		return false;

	cnt = 0;
	do {
		stream_getc(p);
		if (cnt < ident_max) {
			if (ident_out)
				ident_out[cnt] = ch;
			cnt++;
		}
		ch = stream_peekc(p);
	} while (isalnum(ch) || ch == '_');

	if (cnt >= ident_max) {
		stream_set_error(p); /* error: buffer overflow */
		if (ident_out && ident_max)
			*ident_out = 0;
		return false;
	}
	ident_out[cnt] = 0;

	return true;
}
#endif
#endif
