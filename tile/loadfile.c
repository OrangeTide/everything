/* loadfile.c : loads the entire file into a string - public domain. */
#ifndef LOADFILE_H
#define LOADFILE_H

char *load_file(const char *filename, size_t *outlen);

#ifdef LOADFILE_IMPLEMENTATION
#include <stdio.h>

char *
load_file(const char *filename, size_t *outlen)
{
	FILE *f;
	long len;
	char *s = NULL;

	f = fopen(filename, "r");
	if (!f) {
		perror(filename);
		return NULL;
	}

	if (fseek(f, 0, SEEK_END))
		goto failure; /* not seekable? */
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	s = malloc(len + 1);
	if (!s)
		goto failure;
	len = fread(s, 1, len, f);
	if (ferror(f))
		goto failure;
	s[len] = 0;
	if (outlen)
		*outlen = len;

	return s;
failure:
	perror(filename);
	free(s);
	fclose(f);
	return NULL;
}
#endif
#endif
