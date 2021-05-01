#ifndef PROPERTY_H_
#define PROPERTY_H_
#include <stddef.h>

#define PROPERTY_DEFINE(name,value) name "\0" value "\0"

struct property;

struct property *property_make(const char *initial);
void property_free(struct property *prop);
struct property *property_dup(const struct property *prop);
size_t property_size(const struct property *prop);
int property_add(struct property **prop_in_out, const char *name, const char *value);
int property_get(const struct property *prop, const char *name, const char **value_out);
#endif
