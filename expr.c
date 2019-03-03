/* expr.c : evaluates simple arithmetic expressions */
/*
 * Written in 2019 by Jon Mayo <jon@rm-f.net>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

/* # Usage
 *
 * Provide expression as one or more arguments on the command-line. Example:
 *     $ ./expr 1+2-3+4
 *      4
 *
 *     $ ./expr 1+2-3+4
 *      4

 *     $ ./expr 17/20
 *      0.85
 *
 * # Bugs, Missing Features
 *
 * - There is no support for unary minus.
 * - I barely tested anything.
 *
 * # Internal Architecture
 *
 * This implements the [Shunting Yard algorithm][1]. Enjoy!
 *
 * ## Summary
 *
 * 1. If token is a value, then output it.
 * 2. If token is a lparen, then push it onto the operator stack.
 * 3. If token is a rparen, then discard it, pop and output until left paren
 *     found. discard lparen.
 * 4. If token is an operator of lower precedence, pop until this is not true,
 *     push operator onto stack.
 * 5. at the end of expresion, pop and print all operators. parens would be
 *     invalid.
 *
 * ## References
 *
 * [1]: http://www.oxfordmathcenter.com/drupal7/node/628
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#define verbose 0
/* enable verbose mode to see the stack operations performed */
/* #define verbose 1 */

static signed char ostack[64]; /* operator stack - used during parsing */
static unsigned ostack_len;

static double dstack[128]; /* data stack - used during calculation */
static unsigned dstack_len;

/* simple suite of stack macros */
#define stack_max(stack) (sizeof(stack) / sizeof*(stack))
#define stack_init(stack) do { (stack##_len) = stack_max(stack); } while (0)
#define stack_checkpush(stack) ((stack##_len) > 0)
#define stack_checkpop(stack) ((stack##_len) < stack_max(stack))
#define stack_push(stack, value) ((stack)[--(stack##_len)] = (value))
#define stack_pop(stack) ((stack)[(stack##_len)++])
#define stack_peek(stack) ((stack)[(stack##_len)])

static void
report(const char *m)
{
	fprintf(stderr, "%s\n", m);
}

static void
print_op(char op)
{
	fprintf(stdout, " %c", op);
}

static void
print_value(double v)
{
	fprintf(stdout, " %g", v);
}

/* loads value into the run-time */
static int
exec_value(double v)
{
	if (verbose)
		print_value(v);

	if (!stack_checkpush(dstack))
		return -1;

	stack_push(dstack, v);

	return 0;
}

static int
exec_op(char op)
{
	double a, b;

	if (verbose)
		print_op(op);


	if (!stack_checkpop(dstack))
		return -1;
	b = stack_pop(dstack);
	if (!stack_checkpop(dstack))
		return -1;
	a = stack_peek(dstack);

	switch (op) {
	case '+':
		stack_peek(dstack) = a + b;
		break;
	case '-':
		stack_peek(dstack) = a - b;
		break;
	case '*':
		stack_peek(dstack) = a * b;
		break;
	case '/':
		stack_peek(dstack) = a / b;
		break;
	case '%':
		stack_peek(dstack) = fmod(a, b);
		break;
	case '^':
		stack_peek(dstack) = pow(a, b);
		break;
	case '(': case ')':
		return report("unmatched paren"),-1;
	}

	return 0;
}

/* determine precedence of an operator */
static int
prec(char op)
{
	if (op == '+' || op == '-')
		return 1;
	if (op == '*' || op == '/' || op == '%')
		return 2;
	if (op == '^')
		return 3;

	return -1;
}

static int
op_is_left(char op)
{
	if (op == '^') /* right associative */
		return 0;
	return 1;
}

static int
do_oper(const char *s, const char **out)
{
	char new_op = *s++;
	int new_prec = prec(new_op);

	while (stack_checkpop(ostack) && (prec(stack_peek(ostack)) > new_prec ||
			(op_is_left(new_op) &&
			prec(stack_peek(ostack)) == new_prec))) {
		int op = stack_pop(ostack);

		exec_op(op); /* output operator to run-time engine */
	}
	if (!stack_checkpush(ostack))
		return report("stack error"),-1;
	stack_push(ostack, new_op);

	if (out)
		*out = s;

	return 0;
}

static int
parse(const char *s)
{
	while (*s) {
		char ch;

		while (isspace(*s))
			s++;

		ch = *s;

		if (isdigit(ch) || ch == '.') { /* parse a number */
			double v;
			char *e;

			errno = 0;
			v = strtod(s, &e);
			if (e == s || errno)
				return report("numeric conversion"),-1;
			exec_value(v); /* output to run-time engine */
			s = e;
		} else if (isalpha(ch) || ch == '_') { /* parse an identifier */
			const char *e;

			for(e = s; isalnum(*e) || *e == '_' || *e == '$'; e++)
				;
			fprintf(stderr, " FIXME: %.*s", (int)(e - s), s);
			s = e;
		} else if (prec(ch) > 0) { /* parse an operator */
			if (do_oper(s, &s))
				return report("operator error"),-1;
		} else if (ch == '(') {
			stack_push(ostack, '(');
			s++;
		} else if (ch == ')') {
			/* output everything up to next ( */
			while (stack_checkpop(ostack) && stack_peek(ostack) != '(')
				exec_op(stack_pop(ostack));
			if (!stack_checkpop(ostack))
				return -1;
			(void)stack_pop(ostack); /* discard ( */
			s++;
		} else {
			return -1;
		}
	}

	return 0;
}

static void
begin(void)
{
	stack_init(ostack);
	stack_init(dstack);
}

static int
end(void)
{
	/* flush remaining operators from the operator stack */
	while (stack_checkpop(ostack)) {
		int op = stack_pop(ostack);

		if (exec_op(op)) /* output operator to run-time engine */
			return -1;
	}

	if (verbose)
		fprintf(stdout, " ==> \n");

	if (!stack_checkpop(dstack))
		return report("no value"), -1;

	print_value(stack_pop(dstack));

	putchar('\n');

	return 0;
}

int
main(int argc, char **argv)
{
	int i;

	if (argc <= 1)
		return report("usage: expr <expr...>"), 1;

	begin();
	for (i = 1; i < argc; ++i)
		if (parse(argv[i]))
			return report("parse"), 1;
	if (end())
		return 1;

	return 0;
}
