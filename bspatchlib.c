/*-
 * Copyright 2003-2005 Colin Percival
 * Copyright 2007 Andreas John <dynacore@tesla.inka.de>
 * Copyright 2023 Peter Vaskovic <petervaskovic@yahoo.de>
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <bzlib.h>
#include "bspatchlib.h"

#define ERRSTR_MAX_LEN 1024
#define HEADER_SIZE 32

/***************************************************************************/

char *file_to_mem(const char *fname, unsigned char **buf, int *buf_sz)
{
	FILE *f;
	int l_sz;
	unsigned char *l_buf;
	static char errstr[ERRSTR_MAX_LEN];

	*buf = NULL;
	*buf_sz = 0;

	if ((f = fopen(fname, "rb")) == NULL)
	{
		snprintf(errstr, ERRSTR_MAX_LEN, "Cannot open file \"%s\" for reading.", fname);
		return errstr;
	}
	if (fseek(f, 0, SEEK_END) || (l_sz = ftell(f)) <= 0)
	{
		fclose(f);
		snprintf(errstr, ERRSTR_MAX_LEN, "Seek failure on file \"%s\".", fname);
		return errstr;
	}
	if ((l_buf = (unsigned char *)malloc(l_sz + 1)) == NULL)
	{
		fclose(f);
		snprintf(errstr, ERRSTR_MAX_LEN, "Cannot allocate %d bytes to read file \"%s\" into memory.", l_sz + 1, fname);
		return errstr;
	}

	fseek(f, 0, SEEK_SET);

	if ((int)fread(l_buf, 1, l_sz, f) != l_sz)
	{
		free(l_buf);
		fclose(f);
		snprintf(errstr, ERRSTR_MAX_LEN, "Cannot read from file \"%s\".", fname);
		return errstr;
	}
	fclose(f);
	*buf = l_buf;
	*buf_sz = l_sz;
	return NULL;
}

/***************************************************************************/

char *mem_to_file(const unsigned char *buf, int buf_sz, const char *fname)
{
	FILE *f;
	static char errstr[ERRSTR_MAX_LEN];

	if ((f = fopen(fname, "wb")) == NULL)
	{
		snprintf(errstr, ERRSTR_MAX_LEN, "Cannot open file \"%s\" for writing.", fname);
		return errstr;
	}
	if ((int)fwrite(buf, 1, buf_sz, f) != buf_sz)
	{
		fclose(f);
		snprintf(errstr, ERRSTR_MAX_LEN, "Cannot write to file \"%s\".", fname);
		return errstr;
	}
	fclose(f);
	return NULL;
}

/***************************************************************************/

static off_t offtin(const unsigned char *buf)
{
	off_t y;
	y = buf[7] & 0x7F;
	y = y * 256;
	y += buf[6];
	y = y * 256;
	y += buf[5];
	y = y * 256;
	y += buf[4];
	y = y * 256;
	y += buf[3];
	y = y * 256;
	y += buf[2];
	y = y * 256;
	y += buf[1];
	y = y * 256;
	y += buf[0];

	if (buf[7] & 0x80)
	{
		y = -y;
	}

	return y;
}

/***************************************************************************/

static char *decompress_block(const unsigned char *in_buf, int in_sz, unsigned char **out_buf, int *out_sz)
{
	int rc, known_size;
	unsigned sz;
	unsigned char *buf;
	static char errstr[ERRSTR_MAX_LEN];

	known_size = *out_sz;
	*out_buf = NULL;
	*out_sz = 0;

	sz = (known_size >= 0) ? (known_size + 16) : 1024 + 8 * in_sz;
	for (;;)
	{
		if ((buf = (unsigned char *)malloc(sz)) == NULL)
		{
			snprintf(errstr, ERRSTR_MAX_LEN, "decompress_block: cannot allocate %u bytes for patch", sz);
			return errstr;
		}
		rc = BZ2_bzBuffToBuffDecompress((char *)buf, &sz, (char *)in_buf, in_sz, 0, 0);
		if (rc == BZ_OK)
		{
			*out_buf = buf;
			*out_sz = (int)sz;
			return NULL;
		}
		if (rc != BZ_OUTBUFF_FULL)
		{
			snprintf(errstr, ERRSTR_MAX_LEN, "decompress_block: BZ2_bzBuffToBuffDecompress() returned %d", rc);
			return errstr;
		}

		/*
		 * We come to this point if the decompression failed because
		 * we have not allocated enough memory
		 */
		if (known_size >= 0) /* This should not happen, but don't break if it does */
			sz = 1024 + 8 * in_sz;

		if (sz >= 128 * 1024 * 1024) /* Seems to me that 128M should be more than enough for a patch */
			break;
		sz *= 2;
	}
	snprintf(errstr, ERRSTR_MAX_LEN, "decompress_block: even %u bytes are not enough for the patch", sz);
	return errstr;
}

/***************************************************************************/

char *bspatch_mem(const unsigned char *old_buf, int old_size,
				  unsigned char **new_buf, int *new_size,
				  const unsigned char *compr_patch_buf, int compr_patch_buf_sz,
				  int uncompr_ctrl_sz, int uncompr_diff_sz, int uncompr_xtra_sz)
{
	int l_new_size, dec_ctrl_sz, dec_diff_sz, dec_xtra_sz;
	off_t bzctrllen, bzdifflen, bzextralen;
	unsigned char *l_new_buf, *dec_ctrl_buf, *dec_diff_buf, *dec_xtra_buf,
		*pctrl, *pdiff, *pxtra, *pctrl_end, *pdiff_end, *pxtra_end;
	off_t oldpos, newpos, i, ctrl[3];
	char *errmsg;
	static char errstr[ERRSTR_MAX_LEN];

	*new_buf = NULL;
	*new_size = 0;

	/*
	 * File format:
	 *	0	8	"BSDIFF40"
	 *	8	8	X
	 *	16	8	Y
	 *	24	8	sizeof(newfile)
	 *	32	X	bzip2(control block)
	 *	32+X	Y	bzip2(diff block)
	 *	32+X+Y	???	bzip2(extra block)
	 * with control block a set of triples (x,y,z) meaning "add x bytes
	 * from oldfile to x bytes from the diff block; copy y bytes from the
	 * extra block; seek forwards in oldfile by z bytes".
	 */
	if (compr_patch_buf_sz < HEADER_SIZE)
	{
		snprintf(errstr, ERRSTR_MAX_LEN, "Corrupt patch. Too short patch size");
		return errstr;
	}

	/* Check for appropriate magic */
	if (memcmp(compr_patch_buf, "BSDIFF40", 8) != 0)
	{
		snprintf(errstr, ERRSTR_MAX_LEN, "Corrupt patch. Bad header signature");
		return errstr;
	}

	/* Read lengths from header */
	bzctrllen = offtin(compr_patch_buf + 8);
	bzdifflen = offtin(compr_patch_buf + 16);
	l_new_size = offtin(compr_patch_buf + 24);

	if (bzctrllen <= 0 || bzdifflen <= 0 || l_new_size <= 0 || compr_patch_buf_sz <= HEADER_SIZE + bzctrllen + bzdifflen)
	{
		snprintf(errstr, ERRSTR_MAX_LEN, "Corrupt patch. Bad header lengths");
		return errstr;
	}
	bzextralen = compr_patch_buf_sz - HEADER_SIZE - bzctrllen - bzdifflen;

	dec_ctrl_sz = uncompr_ctrl_sz;
	if ((errmsg = decompress_block(compr_patch_buf + HEADER_SIZE, bzctrllen,
								   &dec_ctrl_buf, &dec_ctrl_sz)) != NULL)
		return errmsg;

	dec_diff_sz = uncompr_diff_sz;
	if ((errmsg = decompress_block(compr_patch_buf + HEADER_SIZE + bzctrllen, bzdifflen,
								   &dec_diff_buf, &dec_diff_sz)) != NULL)
	{
		free(dec_ctrl_buf);
		return errmsg;
	}

	dec_xtra_sz = uncompr_xtra_sz;
	if ((errmsg = decompress_block(compr_patch_buf + HEADER_SIZE + bzctrllen + bzdifflen, bzextralen,
								   &dec_xtra_buf, &dec_xtra_sz)) != NULL)
	{
		free(dec_diff_buf);
		free(dec_ctrl_buf);
		return errmsg;
	}

	/* Print decompressed ctrl/diff/xtra sizes */
	if (old_buf == NULL)
	{
		free(dec_xtra_buf);
		free(dec_diff_buf);
		free(dec_ctrl_buf);
		printf("Decompressed ctrl/diff/extra sizes are: %d/%d/%d.", dec_ctrl_sz, dec_diff_sz, dec_xtra_sz);
		return NULL;
	}

	if ((l_new_buf = (unsigned char *)malloc(l_new_size + 1)) == NULL)
	{
		free(dec_xtra_buf);
		free(dec_diff_buf);
		free(dec_ctrl_buf);
		snprintf(errstr, ERRSTR_MAX_LEN, "Cannot allocate %d bytes to create the patch.", l_new_size + 1);
		return errstr;
	}

	pctrl_end = (pctrl = dec_ctrl_buf) + dec_ctrl_sz;
	pdiff_end = (pdiff = dec_diff_buf) + dec_diff_sz;
	pxtra_end = (pxtra = dec_xtra_buf) + dec_xtra_sz;

	oldpos = newpos = 0;

	while (newpos < l_new_size)
	{
		/* Read control data */
		if (pctrl > pctrl_end - 2 * 8)
		{
			snprintf(errstr, ERRSTR_MAX_LEN, "Corrupt patch 1.");
		GETOUT:
			free(dec_xtra_buf);
			free(dec_diff_buf);
			free(dec_ctrl_buf);
			free(l_new_buf);
			return errstr;
		}
		for (i = 0; i <= 2; i++)
		{
			ctrl[i] = offtin(pctrl);
			pctrl += 8;
		}

		/* Sanity-check */
		if ((ctrl[0] < 0) || (ctrl[1] < 0))
		{
			snprintf(errstr, ERRSTR_MAX_LEN, "Corrupt patch 2.");
			goto GETOUT;
		}

		/* Sanity-check */
		if (newpos + ctrl[0] > l_new_size)
		{
			snprintf(errstr, ERRSTR_MAX_LEN, "Corrupt patch 3.");
			goto GETOUT;
		}

		/* Read diff string */
		if (pdiff > pdiff_end - ctrl[0])
		{
			snprintf(errstr, ERRSTR_MAX_LEN, "Corrupt patch 4.");
			goto GETOUT;
		}
		memcpy(l_new_buf + newpos, pdiff, ctrl[0]);
		pdiff += ctrl[0];

		/* Add old data to diff string */
		for (i = 0; i < ctrl[0]; i++)
		{
			if ((oldpos + i >= 0) && (oldpos + i < old_size))
				l_new_buf[newpos + i] += old_buf[oldpos + i];
		}

		/* Adjust pointers */
		newpos += ctrl[0];
		oldpos += ctrl[0];

		/* Sanity-check */
		if (newpos + ctrl[1] > l_new_size)
		{
			snprintf(errstr, ERRSTR_MAX_LEN, "Corrupt patch 5.");
			goto GETOUT;
		}

		/* Read extra string */
		if (pxtra > pxtra_end - ctrl[1])
		{
			snprintf(errstr, ERRSTR_MAX_LEN, "Corrupt patch 6.");
			goto GETOUT;
		}
		memcpy(l_new_buf + newpos, pxtra, ctrl[1]);
		pxtra += ctrl[1];

		/* Adjust pointers */
		newpos += ctrl[1];
		oldpos += ctrl[2];
	}

	/* Clean up the bzip2 reads */
	free(dec_xtra_buf);
	free(dec_diff_buf);
	free(dec_ctrl_buf);

	*new_buf = l_new_buf;
	*new_size = l_new_size;
	return NULL;
}

/***************************************************************************/

char *bspatch(const char *oldfile, const char *newfile, const char *patchfile)
{
	unsigned char *in_buf, *out_buf, *patch_buf;
	int in_sz, out_sz, patch_sz;
	char *errmsg;

	errmsg = file_to_mem(oldfile, &in_buf, &in_sz);
	if (errmsg)
		return errmsg;

	errmsg = file_to_mem(patchfile, &patch_buf, &patch_sz);
	if (errmsg)
	{
		free(in_buf);
		return errmsg;
	}

	errmsg = bspatch_mem(in_buf, in_sz, &out_buf, &out_sz, patch_buf, patch_sz, -1, -1, -1);
	free(in_buf);
	free(patch_buf);
	if (errmsg)
		return errmsg;

	errmsg = mem_to_file(out_buf, out_sz, newfile);
	free(out_buf);
	return (errmsg) ? errmsg : NULL;
}

/***************************************************************************/

char *bspatch_info(const char *patchfile)
{
	unsigned char *dummy_buf, *patch_buf;
	int dummy_sz, patch_sz;
	char *errmsg;

	errmsg = file_to_mem(patchfile, &patch_buf, &patch_sz);
	if (errmsg)
	{
		return errmsg;
	}

	/* Passing NULL as the first argument to bspatch_mem will make it print
	   the decompressed ctrl/diff/xtra sizes of the patch file to the console.
	   These can then be used when calling bspatch_mem to speed up the memory
	   allocation */
	errmsg = bspatch_mem(NULL, 0, &dummy_buf, &dummy_sz, patch_buf, patch_sz, -1, -1, -1);
	free(patch_buf);
	return (errmsg) ? errmsg : NULL;
}

/***************************************************************************/
