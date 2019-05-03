/* basic.c : BASIC interpreter - public domain. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/******************************************************************************/
/* Public Data-types */
/******************************************************************************/

#define LABELS_MAX 2000

typedef struct basic_environment basic_env_t;

/******************************************************************************/
/* Internal Data-types */
/******************************************************************************/

enum commands {
	CMD_PRINT, CMD_INPUT, CMD_IF, CMD_THEN, CMD_GOTO, CMD_FOR, CMD_TO,
	CMD_NEXT, CMD_GOSUB, CMD_RETURN, CMD_END, CMD_LET,
};

enum functions { FUN_ABS, FUN_ATN, FUN_COS, FUN_EXP, FUN_INT, FUN_LOG, FUN_RND,
	FUN_SGN, FUN_SIN, FUN_SQR, FUN_TAN, FUN_FN0, FUN_FN1, FUN_FN2, FUN_FN3,
	FUN_FN4, FUN_FN5, FUN_FN6, FUN_FN7, FUN_FN8, FUN_FN9
};

enum tokens {
	TOK_COMMAND, TOK_EOL, TOK_FINISHED, TOK_NUMBER, TOK_VARIABLE, TOK_STRING,
};

struct basic_environment {
	/* run state */
	double var[26];
	unsigned label_len;
	struct label_entry {
		unsigned linenum;
		unsigned ofs;
	} label_table[LABELS_MAX];
	unsigned current_line;
	/* source code */
	char *source;
	size_t source_len, source_max;
	/* parse state */
	enum tokens token_type;
	union {
		char token[80]; /* number, string, identifier, ... */
		enum commands cmd;
		enum functions fun;
	} token;
//	unsigned line_num;
};

/******************************************************************************/
/* Internal functions */
/******************************************************************************/

/* appends a string to a buffer, growing allocation.
 * returns offset of original string on success, -1 on error */
static int
append(const char *data, size_t data_len, char **base, size_t *len, size_t *max)
{
	size_t n = *len;
	size_t m = *max;

	if (n + data_len + 1 >= m) {
		if (!m)
			m = 16;
		else
			m *= 2;
		char *new = realloc(*base, m);
		if (!new)
			return -1;
		*base = new;
		*max = m;
	}

	char *p = *base + n;
	memcpy(p, data, data_len);
	p[data_len] = 0;
	*len = n + data_len + 1;

	return n;
}

static int
line_add(basic_env_t *env, const char *line, unsigned linenum)
{
	int ofs;
	ofs = append(line, strlen(line), &env->source, &env->source_len,
		     &env->source_max);
	if (ofs < 0)
		return -1; /* could not add */

	struct label_entry *ent = &env->label_table[env->label_len++];
	ent->linenum = linenum;
	ent->ofs = ofs;

	return 0;
}

static int
line_rewind(basic_env_t *env)
{
	if (!env->label_len)
		return -1;
	env->current_line = 0;
	return 0;
}

static const char *
line_next(basic_env_t *env)
{
	if (env->current_line >= env->label_len)
		return NULL;
	unsigned i = env->current_line++;
	return env->source + env->label_table[i].ofs;
}

/* look up matching label in label_table. returns slot in table */
static int
lookup(basic_env_t *env, unsigned linenum)
{
	unsigned i, max = env->label_len;
	for (i = 0; i < max; i++) {
		if (env->label_table[i].linenum == linenum)
			return i;
	}
	return -1;
}

/* Format of info is: <length><string><length><string> ... NUL */
static int
generic_parser(const char *s, char **endptr, const char *info)
{
	unsigned pos, len;
	unsigned n;

	for (n = 0, pos = 0; info[pos]; pos += len, n++) {
		len = info[pos++] - '0';
		if (strncasecmp(s, info + pos, len) == 0) {
			if (endptr)
				*endptr = (char*)s + len;
			return n;
		}
	}

	if (endptr)
		*endptr = (char*)s;

	return -1; /* no match */
}

/* parse for built-in keywords.
 * return -1 on failure.
 * returns enum keyword cast to int on success.
 */
static int
parse_keyword(const char *s, char **endptr)
{
	/* must be in the same order as enum keyword */
	const char keywords[] = "5PRINT5INPUT2IF4THEN4GOTO3FOR2TO4NEXT5GOSUB6RETURN3END3LET";

	return generic_parser(s, endptr, keywords);
}

/* parse for built-in functions.
 * return -1 on failure.
 * returns enum function cast to int on success.
 */
static int
parse_function(const char *s, char **endptr)
{
	/* must be in the same order as enum keyword */
	const char functions[] = "3ABS3ATN3COS3EXP3INT3LOG3RND3SGN3SIN3SQR3TAN";

	/* check for FNx */
	if (toupper(s[0]) == 'F' && toupper(s[1]) == 'N' && isdigit(s[2]))
		return FUN_FN0 + s[2] - '0';

	return generic_parser(s, endptr, functions);
}

static int
execute(basic_env_t *env, const char *line)
{
	fprintf(stderr, "TODO: %s\n", line);
	// TODO: implement this
	return -1;
}

static int
parse_line(basic_env_t *env, const char *line)
{
	char *end;
	int linenum;

	errno = 0;
	linenum = strtoul(line, &end, 10);
	if (end != line) {
		/* add line */
		while (isspace(*end))
			end++;
		line = end;

		return line_add(env, line, linenum);
	} else {
		/* interpret immediately */
		fprintf(stderr, "TRACE:%s():%d\n", __func__, __LINE__);

		return execute(env, line);
	}
}

/******************************************************************************/
/* Top-level API */
/******************************************************************************/

void
basic_free(basic_env_t *env)
{
	// TODO: implement this
	free(env);
}

int
basic_run(basic_env_t *env)
{
	const char *line;

	line_rewind(env);

	while ((line = line_next(env))) {
		if (execute(env, line))
			return -1;
	}

	return 0;
}

// TODO: replace this with a line-at-a-time version
int
basic_load(basic_env_t *env, const char *prog)
{
	const char *cur, *end;

	/* decompose giant string into lines.
	 * TODO: handle CR/LF.
	 */
	for (cur = prog; *cur; cur = end) {
		end = strchr(cur, '\n');
		if (!end)
			end = cur + strlen(cur) - 1;
		int len = end - cur;
		end++;
		char line[len + 1];
		memcpy(line, cur, len);
		line[len] = 0;
		if (parse_line(env, line))
			return -1;
	}

	return 0; /* success */
}

int
basic_init(basic_env_t **env_out)
{
	if (!env_out)
		return -1;
	basic_env_t *env = calloc(1, sizeof(*env));
	if (!env)
		return -1;
	*env_out = env;
	return 0;
}

// TODO: basic_step()

/******************************************************************************/
/* Main */
/******************************************************************************/

static int
loadfile(const char *filename, char **buf_out, size_t *len_out)
{
	FILE *f = fopen(filename, "r");
	if (!f) {
		perror(filename);
		return -1;
	}
	fseek(f, 0L, SEEK_END);
	long n = ftell(f);
	rewind(f);

	char *b = malloc(n + 1);
	if ((long)fread(b, 1, n, f) != n) {
		perror(filename);
		free(b);
		fclose(f);
		return -1;
	}
	fclose(f);

	if (buf_out) {
		*buf_out = b;
	} else {
		free(b);
	}

	if (len_out)
		*len_out = n;

	return 0;
}

/* simple program that accepts a list of file(s) to execute */
int
main(int argc, char **argv)
{
	if (argc <= 1) {
		fprintf(stderr, "usage: %s [programs...]\n", argv[0]);
		// TODO: implement an interactive mode
		return 1;
	}

	int i;
	for (i = 1; i < argc; i++) {
		char *prog;
		int status;
		const char *fn = argv[i];
		basic_env_t *env;

		basic_init(&env);
		if (loadfile(fn, &prog, NULL)) {
			fprintf(stderr, "%s:unable to open file\n", fn);
			return 1;
		}
		status = basic_load(env, prog);
		free(prog);
		if (status) {
			fprintf(stderr, "%s:unable to parse file\n", fn);
			return 1;
		}
		status = basic_run(env);
		basic_free(env);
		if (status) {
			fprintf(stderr, "%s:terminated unsuccessfully\n", fn);
			return 1;
		}
	}

	return 0;
}

#if 0
#define TRACE(m, ...) fprintf(stderr, "%s():%d:" m "\n", __func__, __LINE__, ## __VA_ARGS__)

/******************************************************************************/
/* Exported types */
/******************************************************************************/

struct basic_env;

/******************************************************************************/
/* Data types */
/******************************************************************************/

struct source_line {
	struct source_line *next; /* linked list */
	unsigned short num;
	unsigned short len;
	char line[];
};

/* interpreter environment */
struct basic_env {
	struct source_line *lines_head;
	struct source_line *current_line; /* current program position */
	int error; /* error flag */
	int colpos; /* colunn position on terminal */
	int screenwidth;
	int tabsize;
	char scratch[128]; /* scratch area for string constants */
};

/******************************************************************************/
/* Internal functions - source line data structure */
/******************************************************************************/

static void
line_free(struct source_line *sl)
{
	free(sl);
}

static struct source_line *
line_new(unsigned linenum, const char *line, int len)
{
	struct source_line *sl;

	if (len < 0)
		len = line ? strlen(line) : 0;

	sl = malloc(sizeof(*sl) + len + 1);
	if (!sl)
		return NULL;

	sl->num = linenum;
	sl->next = NULL;
	sl->len = len;
	if (line) {
		memcpy(sl->line, line, len);
		sl->line[len] = 0;
	} else {
		memset(sl->line, 0, len + 1);
	}

	return sl;
}

/* find where we might insert an entry.
 * returned pointer will be where 'num' should be inserted/replaced. */
static struct source_line **
line_find_prev(struct source_line **head, unsigned num)
{
	struct source_line *cur, **prev;

	for (prev = head, cur = *head; cur != NULL; prev = &cur->next, cur = cur->next) {
		if (num <= cur->num)
			return prev;
	}

	return prev;
}

static struct source_line *
line_find(struct source_line **head, unsigned num)
{
	struct source_line **prev;

	prev = line_find_prev(head, num);
	if (!prev || !*prev)
		return NULL;
	if ((*prev)->num != num)
		return NULL;
	return *prev;
}

/* insert/replace a line */
static int
line_insert(struct source_line **head, struct source_line *sl)
{
	struct source_line **prev, *old;

	TRACE("add num=%d line=\"%s\"", sl->num, sl->line);
	prev = line_find_prev(head, sl->num);
	if (!prev)
		return -1;

	/* there are 3 cases to support:
	 * 1. empty list,
	 * 2. replace duplicate item on list,
	 * 3. insert before next highest item on list.
	 */

	old = *prev;
	if (!old) {
		sl->next = NULL;
		*prev = sl;
		return 0;
	}

	if (old->num == sl->num) { /* replace a duplicate line */
		sl->next = old->next;
		old->next = NULL;
		line_free(old);
		*prev = sl;
	} else { /* insert before */
		assert(sl->num < old->num);
		sl->next = old;
		*prev = sl;
	}

	return 0;
}

/******************************************************************************/
/* Internal functions - parser */
/******************************************************************************/

/// enum keyword { K_PRINT, K_INPUT, K_IF, K_THEN, K_GOTO, K_FOR, K_NEXT, K_GOSUB,
///	K_RETURN, K_END, K_LET, };

/// enum function { F_ABS, F_ATN, F_COS, F_EXP, F_INT, F_LOG, F_RND, F_SGN, F_SIN,
///	F_SQR, F_TAN, F_FN0, F_FN1, F_FN2, F_FN3, F_FN4, F_FN5, F_FN6, F_FN7,
///	F_FN8, F_FN9 };

/// /* Format of info is: <length><string><length><string> ... NUL */
/// static int
/// generic_parser(const char *s, char **endptr, const char *info)
/// {
///	unsigned pos, len;
///	unsigned n;
///
///	for (n = 0, pos = 0; info[pos]; pos += len, n++) {
///		len = info[pos++] - '0';
///		if (strncasecmp(s, info + pos, len) == 0) {
///			if (endptr)
///				*endptr = (char*)s + len;
///			return n;
///		}
///	}
///
///	if (endptr)
///		*endptr = (char*)s;
///
///	return -1; /* no match */
/// }

/// /* parse for built-in keywords.
///  * return -1 on failure.
///  * returns enum keyword cast to int on success.
///  */
/// static int
/// parse_keyword(const char *s, char **endptr)
/// {
///	/* must be in the same order as enum keyword */
///	const char keywords[] = "5PRINT5INPUT2IF4THEN4GOTO3FOR4NEXT5GOSUB6RETURN3END3LET";
///
///	return generic_parser(s, endptr, keywords);
/// }

/// /* parse for built-in functions.
///  * return -1 on failure.
///  * returns enum function cast to int on success.
///  */
/// static int
/// parse_function(const char *s, char **endptr)
/// {
///	/* must be in the same order as enum keyword */
///	const char functions[] = "3ABS3ATN3COS3EXP3INT3LOG3RND3SGN3SIN3SQR3TAN";
///
///	/* check for FNx */
///	if (toupper(s[0]) == 'F' && toupper(s[1]) == 'N' && isdigit(s[2]))
///		return F_FN0 + s[2] - '0';
///
///	return generic_parser(s, endptr, functions);
/// }

/* evalutes numeric expression (floating point) */
static int
eval_numeric_expression(const char *s, char **endptr, double *out)
{
	char *end;
	double num;
	const char *cur;

	cur = s;

	// TODO: implement an expression parser (recursive or shunting yard)

	/* get a single number (floating point) */
	while (isspace(*cur))
		cur++;

	errno = 0;
	num = strtod(cur, &end);
	if (end == cur || errno) {
		if (endptr)
			*endptr = (char*)s;

		return -1;
	}

	if (endptr)
		*endptr = end;
	if (out)
		*out = num;

	return 0;
}

static int
eval_string_expression(const char *s, char **endptr, char *out, unsigned outmax)
{
	const char *cur;
	unsigned len;

	cur = s;

	while (isspace(*cur))
		cur++;

	if (*cur != '"')
		goto failed_conversion;

	cur++;
	len = 0;
	while (*cur != '"') {
		if (!*cur)
			goto failed_conversion;
		if (out && len + 1 < outmax) {
			out[len] = *cur;
			out[len + 1] = 0;
		}
		len++;
		cur++;
	}

	if (endptr)
		*endptr = (char*)cur + 1;

	return 0;
failed_conversion:
	if (endptr)
		*endptr = (char*)s;

	return -1;
}

/* check if printing another field will wrap the screen */
static void
check_colpos(struct basic_env *env, int cnt)
{
	if (cnt >= env->screenwidth)
		return; /* ignore really long strings */
	if (env->colpos + cnt > env->screenwidth) {
		env->colpos = 0;
		putchar('\n');
	}
}

/* parse and execute a PRINT statement */
static int
exec_stmt_print(struct basic_env *env, const char *args)
{
	const char *cur;
	char *end;
	int result;
	double num;
	int last_print_separator = 0;

	cur = args;

	/* discard optional leading spaces */
	while (isspace(*cur))
		cur++;

	while (*cur) {
		TRACE("running...");

		last_print_separator = 0;

		if (eval_numeric_expression(cur, &end, &num) == 0) {
			/* print a number */
			check_colpos(env, 12); // TODO: calculate number's length
			result = printf(" %.6g", num);
			if (result > 0)
				env->colpos += result;
		} else if (eval_string_expression(cur, &end, env->scratch, sizeof(env->scratch)) == 0) {
			/* print a string */
			check_colpos(env, strlen(env->scratch));
			result = printf("%s", env->scratch);
			if (result > 0)
				env->colpos += result;
		} else {
			return -1; /* failure */
		}

		cur = end;

		/* discard optional spaces before ; */
		while (isspace(*cur))
			cur++;

		if (*cur == ';') {
			last_print_separator = 1;

			cur++;
		} else if (*cur == ',') {
			int rem;

			last_print_separator = 1;

			cur++;

			rem = env->colpos % env->tabsize;
			if (rem) {
				while (rem < env->tabsize)  {
					rem++;
					env->colpos++;
					putchar(' ');
				}
			}
		}

		/* discard optional trailing spaces */
		while (isspace(*cur))
			cur++;
	}

	if (!last_print_separator) {
		env->colpos = 0;
		putchar('\n');
	}

	return 0;
}

/* parse and execute an INPUT statement */
static int
exec_stmt_input(struct basic_env *env, const char *args)
{
	// TODO: parse arguments for INPUT
	printf("TODO: read input\n");

	return 0;
}

/******************************************************************************/
/* Internal functions - interpreter */
/******************************************************************************/

/* add a null-terminated line to the environment */
static int
env_add_line(struct basic_env *env, int linenum, const char *line)
{
	struct source_line *sl;

	sl = line_new(linenum, line, -1);

	if (line_insert(&env->lines_head, sl)) {
		line_free(sl);
		return -1;
	}

	return 0;
}

static int
env_execute(struct basic_env *env, const char *line)
{
	const char *cur;
	char *end;
	int result;

	cur = line;
	/* discard leading spaces */
	while (isspace(*cur))
		cur++;

	TRACE("execute line=\"%s\"", line);
	result = parse_keyword(cur, &end);
	if (result == -1) {
		// TODO: check to see if this is really an assignment statement (LET without LET)
		return -1; /* invalid */
	}

	switch ((enum keyword)result) {
	case K_PRINT:
		return exec_stmt_print(env, end);
	case K_INPUT:
		return exec_stmt_input(env, end);
	default:
		return -1; /* unknown */
	}

	// TODO: interprete a line

	return 0;
}

/******************************************************************************/
/* Exported functions */
/******************************************************************************/

void
basic_free(struct basic_env *env)
{
	if (!env)
		return;

	/* free all the lines */
	while (env->lines_head) {
		struct source_line *cur = env->lines_head;

		env->lines_head = cur->next;
		cur->next = NULL;
		line_free(cur);
	}

	/* free the outer structure */
	free(env);
}

/* allocates a new interpreter environment.
 * return NULL on failure. */
struct basic_env *
basic_new(void)
{
	struct basic_env *env;

	env = calloc(1, sizeof(*env));

	env->screenwidth = 80; // TODO: auto-detect
	env->tabsize = 12; /* big enough for: -1.234567e+89 */
	env->colpos = 0;

	return env;
}

/* parses a line into the environment.
 * return 0 on success, -1 on failure. */
int
basic_line(struct basic_env *env, const char *line)
{
	const char *cur;
	char *end;
	int linenum;

	cur = line;
	errno = 0;
	linenum = strtoul(cur, &end, 10);
	if (end != cur) {
		/* add line */
		while (isspace(*end))
			end++;
		cur = end;

		env_add_line(env, linenum, cur);
	} else {
		/* interpret immediately */

		env_execute(env, cur);
	}

	return 0;
}

/* step through the program a line at a time.
 * return 1 if there are more lines.
 * return 0 if there are no more lines.
 * return -1 on error.
 */
int
basic_step(struct basic_env *env)
{
	struct source_line *cur;

	cur = env->current_line;

	/* initialize as a new program if there is no current line */
	if (!cur) {
		// TODO: initialize variables
		cur = env->lines_head;
	}

	if (cur) {
		env->current_line = cur->next; /* update current line */
		if (env_execute(env, cur->line)) {
			return -1;
		}
	}

	return env->current_line ? 1 : 0;
}

void
basic_reset(struct basic_env *env)
{
	env->current_line = NULL;
}

int
basic_run(struct basic_env *env)
{
	struct source_line *cur, *next;
	int result;

	basic_reset(env);

	do {
		result = basic_step(env);
	} while (result == 1);

	TRACE("result=%d", result);
	return result;
}

/******************************************************************************/
/* Main */
/******************************************************************************/

int
main(int argc, char **argv)
{
	struct basic_env *env;

	env = basic_new();
	if (!env)
		return 1; /* error */
	// simplistic program
	basic_line(env, "10 PRINT \"===\",1,2,4,5");
	basic_line(env, "20 PRINT 1234567;");
	basic_line(env, "30 PRINT -3.21; +5e5  , \"Hello\" ");
	// more advanced program
	// basic_line(env, "20 PRINT \"What is your name?\"");
	// basic_line(env, "30 INPUT A$");
	basic_line(env, "LIST");
	if (basic_run(env)) {
		TRACE("error"); // TBD
		return 1; /* error */
	}
	basic_free(env);

	return 0;
}
#endif
