#ifndef INTERP_H_
#define INTERP_H_
#include <stdio.h>
#include <stdbool.h>
#define INTERP_ERR (-1)
#define INTERP_OK (0)
struct interp;
struct interp *interp_new(void);
void interp_free(struct interp *in);
int interp_set_input(struct interp *in, const char *filename, FILE *f, bool close_f);
int interp_go(struct interp *in);
#endif
