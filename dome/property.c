#include "property.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define ERR (-1)
#define OK (0)

struct property {
	size_t max;
	char data[];
};

////////////////////////////////////////////////////////////////////////

static size_t
parse_length(const char *s, size_t lim)
{
	const char *p, *e;
	size_t n;

	for (p = s; *p; ) {
		// Name
		e = memchr(p, 0, lim);
		if (!e)
			return 0; // ERROR
		n = e - p + 1;
		p = e;
		if (lim <= n)
			return 0; // ERROR
		lim -= n;
		// Value
		e = memchr(p, 0, lim);
		if (!e)
			return 0; // ERROR
		n = e - p + 1;
		p = e;
		if (lim <= n)
			return 0; // ERROR
		lim -= n;
	}

	return p - s;
}

#if 0 // a bit simpler, but doesn't carefully avoid buffer overflows
static size_t
parse_length(const char *s)
{
	size_t n;
	const char *p;

	/* walk through name,value pairs until name is "" */
	for (p = s; n = strlen(p);) {
		++n;
		p += n;
		n = strlen(p) + 1;
		p += n;
	}

	return p - s;
}
#endif

static struct property *
property_alloc(size_t len)
{
	/* minimum size */
	if (len < 2)
		len = 2;

	size_t sz = sizeof(struct property) + len;
	struct property *prop = malloc(sz);
	if (!prop)
		return NULL;
	prop->max = len;

	/* must properly terminate the list */
	prop->data[0] = 0;
	prop->data[1] = 0;

	return prop;
}

/* reads a null terminated string from a buffer */
static const char *
get_one(const char *data, size_t *lim_in_out, const char **out)
{
	char *p = memchr(data, 0, *lim_in_out);
	if (!p)
		return NULL;
	++p;
	*lim_in_out -= p - data;

	if (out)
		*out = data;

	return p;
}

static const char *
get_record(const char *data, size_t lim, const char **name, const char **value)
{
	data = get_one(data, &lim, name);
	if (!data)
		return NULL;
	data = get_one(data, &lim, value);
	if (!data)
		return NULL;
	return data;
}

// return offset to found entry.
// return value is same as prop->used if not found.
static ssize_t
find(const struct property *prop, const char *name)
{

	const char *rec_name;
	const char *data = prop->data;
	size_t lim = prop->max;
	size_t name_len = strlen(name);

	while (1) {
		const char *cur = get_record(data, lim, &rec_name, NULL);
		size_t pos = cur - data;
		if (!cur || pos >= lim)
			return -1;
		if (!*name)
			return pos; /* empty name = end-of-list */
		if (name_len <= lim && !memcmp(name, rec_name, name_len)) {
			return pos; /* match! */
		}
		lim -= pos;
		data = cur;
	}
}

static int
grow(struct property **prop_in_out, size_t newmax)
{
	struct property *prop = *prop_in_out;
	size_t lim = prop->max;

	if (lim >= newmax)
		return 0; /*  no need to grow */

	/* next power-of-two */
	if (newmax <= 2)
		lim = 2;
	else
		lim = (1ULL << 32) >> __builtin_clz(newmax - 1);

	size_t sz = sizeof(*prop) + lim;
	prop = realloc(prop, sz);
	if (!prop)
		return -1; /* unable to alloc */

	/* success ... */
	prop->max = lim;
	*prop_in_out = prop;

	return 0;
}

static int
append(struct property **prop_in_out, size_t pos, const char *name, const char *value)
{
	size_t name_len = strlen(name) + 1;
	size_t value_len = strlen(value) + 1;

	/* grow if needed */
	size_t record_end = pos + name_len + value_len;
	if (grow(prop_in_out, record_end))
		return -1; /* unable to grow buffer */

	/* copy the record */
	struct property *prop = *prop_in_out;
	char *data = prop->data + pos;
	memcpy(data, name, name_len);
	data += name_len;
	memcpy(data, value, value_len);

	return 0; /* success */
}

////////////////////////////////////////////////////////////////////////

/* be mindful that initial is not a normal C string. it must be terminated with
 * a double nul character "\0". */
struct property *
property_make(const char *initial)
{
	size_t len = parse_length(initial, INT_MAX);
	return property_alloc(len);
}

void
property_free(struct property *prop)
{
	free(prop);
}

struct property *
property_dup(const struct property *prop)
{
	if (!prop)
		return NULL;

	size_t len = property_size(prop);
	if (len < 2)
		return NULL; // invalid

	struct property *newprop = property_alloc(len);
	if (!newprop)
		return NULL;
	memcpy(newprop->data, prop->data, len);
	return newprop;
}

size_t
property_size(const struct property *prop)
{
	return parse_length(prop->data, prop->max);

}

int
property_add(struct property **prop_in_out, const char *name, const char *value)
{
	if (!prop_in_out)
		return -1;

	struct property *prop = *prop_in_out;

	ssize_t pos = find(prop, name);
	if (pos < 0)
		return -1;

	return append(prop_in_out, pos, name, value);
}

int
property_get(const struct property *prop, const char *name, const char **value_out)
{
	ssize_t pos = find(prop, name);
	if (pos < 0) {
		if (value_out)
			*value_out = NULL;
		return -1;
	}

	const char *record = prop->data + pos;
	/* bounds check against entire prop array */
	const char *end_of_name = memchr(record, 0, prop->max - pos);

	/* goto to end of Name field to find Value field */
	if (!end_of_name)
		return ERR;

	const char *value = end_of_name + 1;
	const char *end_of_value = memchr(value, 0, prop->max - pos - (record - value));

	/* bounds check that value is within prop array */
	if (!end_of_value)
		return ERR;

	if (value_out)
		*value_out = value;

	return OK;
}
