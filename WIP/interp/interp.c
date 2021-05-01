#include "interp.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

struct interp {

	struct {
		char filename[64]; /* helpful tag, not the actual filename */
		FILE *f;
		bool close_f;
		unsigned linenum;
		int ch;
	} stream;

	struct {
		enum token_type {
			TOK_EOF,
			TOK_NUM,
			TOK_IDENT,
		} type;
		int linenum; /* where token begins */
		size_t len;
		char buf[512];
	} token;

	struct {
		char description[128];
		int code;
	} error;
};

/**********************************************************************/

static const char ident_first[] = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char ident_second[] = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const char digit[] = "0123456789";
static const char whitespace[] = " \t\r\n";

/**********************************************************************/

static void
set_error(struct interp *in, int errorcode, const char *fmt, ...)
{
	if (errorcode && fmt) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(in->error.description, sizeof(in->error.description), fmt, ap);
		va_end(ap);
	} else { /* INTERP_OK or fmt=NULL */
		in->error.description[0] = 0;
	}

	in->error.code = errorcode;
}

static void
set_token(struct interp *in, enum token_type tok)
{
	in->token.type = tok;
	in->token.linenum = in->stream.linenum;
	in->token.len = 0;
	in->token.buf[0] = 0;
#ifndef NDEBUG
	memset(in->token.buf, 0, sizeof in->token.buf);
#endif
}

static void
token_append(struct interp *in, int ch)
{
	if (in->token.len + 1 < sizeof(in->token.buf)) {
		in->token.buf[in->token.len++] = ch;
		in->token.buf[in->token.len] = 0;
	}
}

/**********************************************************************/

/* return the current character */
static int
stream_ch(struct interp *in)
{
	return in->stream.ch;
}

/* consume current character, load the next. */
static int
stream_next(struct interp *in)
{
	if (in->stream.ch == '\n')
		in->stream.linenum++;
	in->stream.ch = fgetc(in->stream.f);
	return in->stream.ch;
}

static void
stream_close_file(struct interp *in)
{
	if (in->stream.close_f) {
		if (in->stream.f)
			fclose(in->stream.f);
	}

	in->stream.f = NULL;
	in->stream.close_f = false;
	in->stream.linenum = 0;
}

/* initialize stream */
static void
stream_init(struct interp *in, const char *filename, FILE *f, bool close_f)
{
	stream_close_file(in);

	if (f) {
		snprintf(in->stream.filename, sizeof(in->stream.filename), "%s", filename);
		in->stream.f = f;
		in->stream.close_f = close_f;
		in->stream.linenum = 1;
	}

	in->stream.ch = EOF;
	stream_next(in);
}

/**********************************************************************/

static bool
match(const char *class, int c)
{
	if (c < 0)
		return false;
	return strchr(class, c) != NULL;
}

/**********************************************************************/

struct interp *
interp_new(void)
{
	struct interp *in = calloc(1, sizeof(in));

	return in;
}

void
interp_free(struct interp *in)
{
	if (in) {
		stream_close_file(in);

		free(in);
	}
}

int
interp_set_input(struct interp *in, const char *filename, FILE *f, bool close_f)
{
	if (!in)
		return INTERP_ERR;

	stream_init(in, filename, f, close_f);

	return INTERP_OK;
}

/**********************************************************************/

bool
token_try_consume(struct interp *in, int c, enum token_type type, const char *first, const char *second)
{
	if (c == EOF)
		return false;

	if (!match(first, c))
		return false;

	set_token(in, type);
	token_append(in, c);

	while ((c = stream_ch(in)) != EOF) {
		if (!match(second, c)) {
			break;
		}
		token_append(in, c);
		stream_next(in);
	}

	return true;
}

bool
token_try_whitespace(struct interp *in, int c)
{
	if (!match(whitespace, c))
		return false;

	/* no token is generate for whitespace, it's silently
	 * discarded and used primarily to delimitate other
	 * tokens */
	while ((c = stream_ch(in)) != EOF) {
		if (!match(whitespace, c)) {
			break;
		}
		stream_next(in);
	}

	return true;
}

/**********************************************************************/

static int
token_next(struct interp *in)
{
	int c;

	while ((c = stream_ch(in)) != EOF) {
		stream_next(in);

		if (token_try_consume(in, c, TOK_IDENT, ident_first, ident_second))
			return INTERP_OK;

		if (token_try_consume(in, c, TOK_NUM, digit, digit))
			return INTERP_OK;

		if (token_try_whitespace(in, c))
			continue;

		set_error(in, INTERP_ERR, "unknown character or token");
		return INTERP_ERR;
	}

	set_token(in, TOK_EOF);
	return INTERP_OK;
}

/**********************************************************************/

int
interp_go(struct interp *in)
{
	while (token_next(in) == INTERP_OK) {
		printf("type: %d; val=\"%s\"\n", in->token.type, in->token.buf);

		// TODO: implement this

		if (in->token.type == TOK_EOF) {
			break;
		}
	}

	return INTERP_ERR;
}
