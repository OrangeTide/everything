/* test_embed.c : outputs the embedded file. - public domain */

/* TESTING:
 * gcc -o test_embed test_embed.c && ./test_embed | md5sum ; md5sum favicon.ico
 */

#include <stdio.h>
#include "jdm_embed.h"

JDM_EMBED_FILE(favicon_file, "favicon.ico");

int
main()
{
	fprintf(stderr, "length:%zd\n", favicon_file_len);
	fwrite(favicon_file, 1, favicon_file_len, stdout);
	return 0;
}
