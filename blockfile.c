#include <stdio.h>

static FILE *blockf;

void
blockfile_close(void)
{
	if (blockf)
		fclose(blockf);
	blockf = NULL;
}

int
blockfile_open(void)
{
	const char *name = BLOCKFILE_NAME;
	
	if (blockf) {
		BLOCKFILE_REPORTERROR("%s:multiple opens requested", name);
		return -1;
	}
	blockf = fopen(name, "rb");
	if (!blockf) {
		BLOCKFILE_REPORTERROR("%s:%s", name, strerror(errno));
		return -1;
	}
	return 0;
}

int
block_read(unsigned ofs, void *b)
{
	return fread(b, BLOCKFILE_BLOCKSIZE, 1, blockf);
}

int
block_write(unsigned ofs, const void *b)
{
	return fwrite(b, BLOCKFILE_BLOCKSIZE, 1, blockf);
}