/* test_embed.c : outputs the embedded file. - public domain */

/* TESTING:
 * gcc -o test_embed test_embed.c && ./test_embed | md5sum ; md5sum README.md
 */

#include <stdio.h>
#include "jdm_embed.h"

JDM_EMBED_FILE(readme_file, "README.md");

int
main()
{
	fprintf(stderr, "length:%d\n", (int)readme_file_len);
	fwrite(readme_file, 1, readme_file_len, stdout);
	return 0;
}
