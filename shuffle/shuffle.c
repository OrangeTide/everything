#include <stdlib.h>

typedef struct shuffle_state shuffle_state_t;

struct shuffle_state {
	unsigned count, remaining;
	unsigned upper, lower;
	int elm[];
};

/* initializes a set of numbers between [lower..uppper] */
int
shuffle_init(shuffle_state_t **state_out, unsigned lower, unsigned upper)
{
	unsigned i, n = upper - lower + 1;
	shuffle_state_t *state;

	state = malloc(sizeof(*state) + sizeof(*state->elm) * n);
	if (!state)
		return -1;

	state->count = n;
	state->remaining = n;
	state->upper = upper;
	state->lower = lower;

	for (n = 0, i = lower; i <= upper; i++)
		state->elm[n++] = i;

	if (state_out)
		*state_out = state;
	else
		free(state);

	return 0;
}

void
shuffle_free(shuffle_state_t **state_out)
{
	if (state_out) {
		free(*state_out);
		*state_out = NULL;
	}
}

/* remove a random entry from the set, returning it.
 * return -1 when set is empty */
int
shuffle_next_random(shuffle_state_t *state)
{
	if (state->remaining <= 0)
		return -1;
	/* pick a random element */
	unsigned i = rand() % state->remaining;
	int v = state->elm[i];
	/* shotern the list, swap in the last element to fill the hole */
	state->remaining--;
	state->elm[i] = state->elm[state->remaining];

	return v;
}

/* return a value to the set.
 * doesn't have to be in [lower..upper] */
int
shuffle_recycle(shuffle_state_t *state, int val)
{
	if (state->remaining >= state->count)
		return -1; /* overflow */

	// TODO: optionally check for duplicate ifndef NDEBUG

	state->elm[state->remaining++] = val;

	return 0;
}

/// test program
#include <stdio.h>
#include <time.h>
int
main()
{
	unsigned i;
	shuffle_state_t *state;

	srand(time(0));
	shuffle_init(&state, 1, 999);
	/* pick 9 entries from the set */
	for (i = 0; i < 9; i++)
		printf(" %3d\n", shuffle_next_random(state));
	/* add a new entry, then pick a final random one */
	shuffle_recycle(state, 1000);
		printf(" %3d\n", shuffle_next_random(state));
	shuffle_free(&state);
	return 0;
}
