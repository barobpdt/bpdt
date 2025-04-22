#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LZ77LIB.h"

#ifdef USE_MAKE_SIGN_LOG
extern void OutputLog( const char *format,...);
#endif 

int CLZ77Lib::lz77_hash(unsigned char *data)
{
    return (257 * data[0] + 263 * data[1] + 269 * data[2]) % HASHMAX;
}

//int CLZ77Lib::lz77_init(struct LZ77Context *ctx)
//int CLZ77Lib::lz77_init(struct CLZ77Lib::LZ77Context *ctx)
int CLZ77Lib::lz77_init(CLZ77Lib::LZ77Context *ctx)
{
    struct CLZ77Lib::LZ77InternalContext *st;
    int i;

    st = (struct CLZ77Lib::LZ77InternalContext *) malloc(sizeof(*st));
    if (!st)
	return 0;

    ctx->ictx = st;

    for (i = 0; i < WINSIZE; i++)
	st->win[i].next = st->win[i].prev = st->win[i].hashval = INVALID;
    for (i = 0; i < HASHMAX; i++)
	st->hashtab[i].first = INVALID;
    st->winpos = 0;

    st->npending = 0;

    return 1;
}

void CLZ77Lib::lz77_advance(struct CLZ77Lib::LZ77InternalContext *st, unsigned char c, int hash)
{
    int off;

    /*
     * Remove the hash entry at winpos from the tail of its chain,
     * or empty the chain if it's the only thing on the chain.
     */
    if (st->win[st->winpos].prev != INVALID) {
	st->win[st->win[st->winpos].prev].next = INVALID;
    } else if (st->win[st->winpos].hashval != INVALID) {
	st->hashtab[st->win[st->winpos].hashval].first = INVALID;
    }

    /*
     * Create a new entry at winpos and add it to the head of its
     * hash chain.
     */
    st->win[st->winpos].hashval = hash;
    st->win[st->winpos].prev = INVALID;
    off = st->win[st->winpos].next = st->hashtab[hash].first;
    st->hashtab[hash].first = st->winpos;
    if (off != INVALID)
	st->win[off].prev = st->winpos;
    st->data[st->winpos] = c;

    /*
     * Advance the window pointer.
     */
    st->winpos = (st->winpos + 1) & (WINSIZE - 1);
}

#define CHARAT(k) ( (k)<0 ? st->data[(st->winpos+k)&(WINSIZE-1)] : data[k] )

void CLZ77Lib::lz77_compress(struct CLZ77Lib::LZ77Context *ctx,  unsigned char *data, int len, int compress)
{
    struct CLZ77Lib::LZ77InternalContext *st = ctx->ictx;
    int i, hash, distance, off, nmatch, matchlen, advance;
    struct CLZ77Lib::Match defermatch, matches[MAXMATCH];
    int deferchr;

    /*
     * Add any pending characters from last time to the window. (We
     * might not be able to.)
     */
    for (i = 0; i < st->npending; i++) {
	unsigned char foo[HASHCHARS];
	int j;
	if (len + st->npending - i < HASHCHARS) {
	    /* Update the pending array. */
	    for (j = i; j < st->npending; j++)
		st->pending[j - i] = st->pending[j];
	    break;
	}
	for (j = 0; j < HASHCHARS; j++)
	    foo[j] = (i + j < st->npending ? st->pending[i + j] :
		      data[i + j - st->npending]);
	lz77_advance(st, foo[0], lz77_hash(foo));
    }
    st->npending -= i;

    defermatch.len = 0;
    deferchr = '\0';
    while (len > 0) 
	{

		/* Don't even look for a match, if we're not compressing. */
		if (compress && len >= HASHCHARS) {
			/*
			 * Hash the next few characters.
			 */
			hash = lz77_hash(data);

			/*
			 * Look the hash up in the corresponding hash chain and see
			 * what we can find.
			 */
			nmatch = 0;
			for (off = st->hashtab[hash].first;
			 off != INVALID; off = st->win[off].next) {
			/* distance = 1       if off == st->winpos-1 */
			/* distance = WINSIZE if off == st->winpos   */
			distance =
				WINSIZE - (off + WINSIZE - st->winpos) % WINSIZE;
			for (i = 0; i < HASHCHARS; i++)
				if (CHARAT(i) != CHARAT(i - distance))
				break;
			if (i == HASHCHARS) {
				matches[nmatch].distance = distance;
				matches[nmatch].len = 3;
				if (++nmatch >= MAXMATCH)
				break;
			}
			}
		} else {
			nmatch = 0;
			hash = INVALID;
		}

		if (nmatch > 0) {
			/*
			 * We've now filled up matches[] with nmatch potential
			 * matches. Follow them down to find the longest. (We
			 * assume here that it's always worth favouring a
			 * longer match over a shorter one.)
			 */
			matchlen = HASHCHARS;
			while (matchlen < len) {
			int j;
			for (i = j = 0; i < nmatch; i++) {
				if (CHARAT(matchlen) ==
				CHARAT(matchlen - matches[i].distance)) {
				matches[j++] = matches[i];
				}
			}
			if (j == 0)
				break;
			matchlen++;
			nmatch = j;
			}

			/*
			 * We've now got all the longest matches. We favour the
			 * shorter distances, which means we go with matches[0].
			 * So see if we want to defer it or throw it away.
			 */
			matches[0].len = matchlen;
			if (defermatch.len > 0) {
			if (matches[0].len > defermatch.len + 1) {
				/* We have a better match. Emit the deferred char,
				 * and defer this match. */
				//ctx->literal(ctx, (unsigned char) deferchr);
					zlib_literal(ctx, (unsigned char) deferchr);
				defermatch = matches[0];
				deferchr = data[0];
				advance = 1;
			} else {
				/* We don't have a better match. Do the deferred one. */
				//ctx->match(ctx, defermatch.distance, defermatch.len);
					zlib_match(ctx, defermatch.distance, defermatch.len);
				advance = defermatch.len - 1;
				defermatch.len = 0;
			}
			} else {
			/* There was no deferred match. Defer this one. */
			defermatch = matches[0];
			deferchr = data[0];
			advance = 1;
			}
		} else {
			/*
			 * We found no matches. Emit the deferred match, if
			 * any; otherwise emit a literal.
			 */
			if (defermatch.len > 0) {
			//ctx->match(ctx, defermatch.distance, defermatch.len);
				zlib_match(ctx, defermatch.distance, defermatch.len);
			advance = defermatch.len - 1;
			defermatch.len = 0;
			} else {
			//ctx->literal(ctx, data[0]);
				zlib_literal(ctx, data[0]);
			advance = 1;
			}
		}

		/*
		 * Now advance the position by `advance' characters,
		 * keeping the window and hash chains consistent.
		 */
		while (advance > 0) {
			if (len >= HASHCHARS) {
			lz77_advance(st, *data, lz77_hash(data));
			} else {
			st->pending[st->npending++] = *data;
			}
			data++;
			len--;
			advance--;
		}
    }
}

/* ----------------------------------------------------------------------
 * Zlib compression. We always use the static Huffman tree option.
 * Mostly this is because it's hard to scan a block in advance to
 * work out better trees; dynamic trees are great when you're
 * compressing a large file under no significant time constraint,
 * but when you're compressing little bits in real time, things get
 * hairier.
 * 
 * I suppose it's possible that I could compute Huffman trees based
 * on the frequencies in the _previous_ block, as a sort of
 * heuristic, but I'm not confident that the gain would balance out
 * having to transmit the trees.
 */

void CLZ77Lib::outbits(struct Outbuf *out, unsigned long bits, int nbits)
{
    // Log("LZ77>> %d\n",out->noutbits + nbits <= 32);
    out->outbits |= bits << out->noutbits;
    out->noutbits += nbits;
    while (out->noutbits >= 8)
	{
		if (out->outlen >= out->outsize) {
			out->outsize = out->outlen + 64;
			out->outbuf = (unsigned char*)realloc(out->outbuf, out->outsize);
		}
		out->outbuf[out->outlen++] = (unsigned char) (out->outbits & 0xFF);
		out->outbits >>= 8;
		out->noutbits -= 8;
    }
}

const unsigned char CLZ77Lib::mirrorbytes[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

const CLZ77Lib::CODERECORD CLZ77Lib::lencodes[] = {
    {257, 0, 3, 3},
    {258, 0, 4, 4},
    {259, 0, 5, 5},
    {260, 0, 6, 6},
    {261, 0, 7, 7},
    {262, 0, 8, 8},
    {263, 0, 9, 9},
    {264, 0, 10, 10},
    {265, 1, 11, 12},
    {266, 1, 13, 14},
    {267, 1, 15, 16},
    {268, 1, 17, 18},
    {269, 2, 19, 22},
    {270, 2, 23, 26},
    {271, 2, 27, 30},
    {272, 2, 31, 34},
    {273, 3, 35, 42},
    {274, 3, 43, 50},
    {275, 3, 51, 58},
    {276, 3, 59, 66},
    {277, 4, 67, 82},
    {278, 4, 83, 98},
    {279, 4, 99, 114},
    {280, 4, 115, 130},
    {281, 5, 131, 162},
    {282, 5, 163, 194},
    {283, 5, 195, 226},
    {284, 5, 227, 257},
    {285, 0, 258, 258},
};

const CLZ77Lib::CODERECORD CLZ77Lib::distcodes[] = {
    {0, 0, 1, 1},
    {1, 0, 2, 2},
    {2, 0, 3, 3},
    {3, 0, 4, 4},
    {4, 1, 5, 6},
    {5, 1, 7, 8},
    {6, 2, 9, 12},
    {7, 2, 13, 16},
    {8, 3, 17, 24},
    {9, 3, 25, 32},
    {10, 4, 33, 48},
    {11, 4, 49, 64},
    {12, 5, 65, 96},
    {13, 5, 97, 128},
    {14, 6, 129, 192},
    {15, 6, 193, 256},
    {16, 7, 257, 384},
    {17, 7, 385, 512},
    {18, 8, 513, 768},
    {19, 8, 769, 1024},
    {20, 9, 1025, 1536},
    {21, 9, 1537, 2048},
    {22, 10, 2049, 3072},
    {23, 10, 3073, 4096},
    {24, 11, 4097, 6144},
    {25, 11, 6145, 8192},
    {26, 12, 8193, 12288},
    {27, 12, 12289, 16384},
    {28, 13, 16385, 24576},
    {29, 13, 24577, 32768},
};

void CLZ77Lib::zlib_literal(struct CLZ77Lib::LZ77Context *ectx, unsigned char c)
{
    struct CLZ77Lib::Outbuf *out = (struct CLZ77Lib::Outbuf *) ectx->userdata;

    if (out->comp_disabled) {
		/*
		 * We're in an uncompressed block, so just output the byte.
		 */
		outbits(out, c, 8);
		return;
    }

    if (c <= 143) {
		/* 0 through 143 are 8 bits long starting at 00110000. */
		outbits(out, mirrorbytes[0x30 + c], 8);
    } else {
		/* 144 through 255 are 9 bits long starting at 110010000. */
		outbits(out, 1 + 2 * mirrorbytes[0x90 - 144 + c], 9);
    }
}

void CLZ77Lib::zlib_match(struct CLZ77Lib::LZ77Context *ectx, int distance, int len)
{
    const CLZ77Lib::CODERECORD *d, *l;
    int i, j, k;
    struct CLZ77Lib::Outbuf *out = (struct CLZ77Lib::Outbuf *) ectx->userdata;

    // Log("LZ77>> %d\n",!out->comp_disabled);

    while (len > 0)
	{
		int thislen;

		/*
		 * We can transmit matches of lengths 3 through 258
		 * inclusive. So if len exceeds 258, we must transmit in
		 * several steps, with 258 or less in each step.
		 * 
		 * Specifically: if len >= 261, we can transmit 258 and be
		 * sure of having at least 3 left for the next step. And if
		 * len <= 258, we can just transmit len. But if len == 259
		 * or 260, we must transmit len-3.
		 */
		thislen = (len > 260 ? 258 : len <= 258 ? len : len - 3);
		len -= thislen;

		/*
		 * Binary-search to find which length code we're
		 * transmitting.
		 */
		i = -1;
		j = sizeof(lencodes) / sizeof(*lencodes);
		while (1) {
			// Log("LZ77>> %d\n",j - i >= 2);
			k = (j + i) / 2;
			if (thislen < lencodes[k].min)
			j = k;
			else if (thislen > lencodes[k].max)
			i = k;
			else {
			l = &lencodes[k];
			break;		       /* found it! */
			}
		}

		/*
		 * Transmit the length code. 256-279 are seven bits
		 * starting at 0000000; 280-287 are eight bits starting at
		 * 11000000.
		 */
		if (l->code <= 279) {
			outbits(out, mirrorbytes[(l->code - 256) * 2], 7);
		} else {
			outbits(out, mirrorbytes[0xc0 - 280 + l->code], 8);
		}

		/*
		 * Transmit the extra bits.
		 */
		if (l->extrabits)
			outbits(out, thislen - l->min, l->extrabits);

		/*
		 * Binary-search to find which distance code we're
		 * transmitting.
		 */
		i = -1;
		j = sizeof(distcodes) / sizeof(*distcodes);
		while (1) {
			// Log("LZ77>> %d\n",j - i >= 2);
			k = (j + i) / 2;
			if (distance < distcodes[k].min)
			j = k;
			else if (distance > distcodes[k].max)
			i = k;
			else {
			d = &distcodes[k];
			break;		       /* found it! */
			}
		}

		/*
		 * Transmit the distance code. Five bits starting at 00000.
		 */
		outbits(out, mirrorbytes[d->code * 8], 5);

		/*
		 * Transmit the extra bits.
		 */
		if (d->extrabits)
			outbits(out, distance - d->min, d->extrabits);
    }
}

void CLZ77Lib::zlib_compress_init(void)
{
    struct CLZ77Lib::Outbuf *out;

	memset(&m_ectx,0,sizeof(m_ectx));

    lz77_init(&m_ectx);
    //m_ectx.literal = zlib_literal;
    //m_ectx.match = zlib_match;

    out = (struct CLZ77Lib::Outbuf *)malloc(sizeof(struct CLZ77Lib::Outbuf));
    out->outbits = out->noutbits = 0;
    out->firstblock = 1;
    out->comp_disabled = FALSE;
    m_ectx.userdata = out;

 //   logevent("Initialised zlib (RFC1950) compression\n");
}

/*
 * Turn off actual LZ77 analysis for one block, to facilitate
 * construction of a precise-length IGNORE packet. Returns the
 * length adjustment (which is only valid for packets < 65536
 * bytes, but that seems reasonable enough).
 */
int CLZ77Lib::Disable_Compression(void)
{
    struct CLZ77Lib::Outbuf *out = (struct CLZ77Lib::Outbuf *) m_ectx.userdata;
    int n;

    out->comp_disabled = TRUE;

    n = 0;
    /*
     * If this is the first block, we will start by outputting two
     * header bytes, and then three bits to begin an uncompressed
     * block. This will cost three bytes (because we will start on
     * a byte boundary, this is certain).
     */
    if (out->firstblock) {
		n = 3;
    } else {
		/*
		 * Otherwise, we will output seven bits to close the
		 * previous static block, and _then_ three bits to begin an
		 * uncompressed block, and then flush the current byte.
		 * This may cost two bytes or three, depending on noutbits.
		 */
		n += (out->noutbits + 10) / 8;
    }

    /*
     * Now we output four bytes for the length / ~length pair in
     * the uncompressed block.
     */
    n += 4;

    return n;
}

int CLZ77Lib::zlib_compress_block(unsigned char *block, int len,	unsigned char **outblock, int *outlen)
{
    struct CLZ77Lib::Outbuf *out = (struct CLZ77Lib::Outbuf *) m_ectx.userdata;
    int in_block;

    out->outbuf = NULL;
    out->outlen = out->outsize = 0;

    /*
     * If this is the first block, output the Zlib (RFC1950) header
     * bytes 78 9C. (Deflate compression, 32K window size, default
     * algorithm.)
     */
    if (out->firstblock) {
		outbits(out, 0x9C78, 16);
		out->firstblock = 0;
		
		in_block = FALSE;
    } else
		in_block = TRUE;
	
    if (out->comp_disabled)
	{
		if (in_block)
			outbits(out, 0, 7);	       /* close static block */

		while (len > 0) {
			int blen = (len < 65535 ? len : 65535);

			/*
			 * Start a Deflate (RFC1951) uncompressed block. We
			 * transmit a zero bit (BFINAL=0), followed by a zero
			 * bit and a one bit (BTYPE=00). Of course these are in
			 * the wrong order (00 0).
			 */
			outbits(out, 0, 3);

			/*
			 * Output zero bits to align to a byte boundary.
			 */
			if (out->noutbits)
			outbits(out, 0, 8 - out->noutbits);

			/*
			 * Output the block length, and then its one's
			 * complement. They're little-endian, so all we need to
			 * do is pass them straight to outbits() with bit count
			 * 16.
			 */
			outbits(out, blen, 16);
			outbits(out, blen ^ 0xFFFF, 16);

			/*
			 * Do the `compression': we need to pass the data to
			 * lz77_compress so that it will be taken into account
			 * for subsequent (distance,length) pairs. But
			 * lz77_compress is passed FALSE, which means it won't
			 * actually find (or even look for) any matches; so
			 * every character will be passed straight to
			 * zlib_literal which will spot out->comp_disabled and
			 * emit in the uncompressed format.
			 */
			lz77_compress(&m_ectx, block, blen, FALSE);

			len -= blen;
			block += blen;
		}
		outbits(out, 2, 3);	       /* open new block */

    } else {
	
		if (!in_block) {
			/*
			 * Start a Deflate (RFC1951) fixed-trees block. We
			 * transmit a zero bit (BFINAL=0), followed by a zero
			 * bit and a one bit (BTYPE=01). Of course these are in
			 * the wrong order (01 0).
			 */
			outbits(out, 2, 3);
		}

		/*
		 * Do the compression.
		 */
#ifdef USE_MAKE_SIGN_LOG
		Output// Log("start - lz77_compress");
#endif

		lz77_compress(&m_ectx, block, len, TRUE);

#ifdef USE_MAKE_SIGN_LOG
		Output// Log("end - lz77_compress");
#endif
		/*
		 * End the block (by transmitting code 256, which is
		 * 0000000 in fixed-tree mode), and transmit some empty
		 * blocks to ensure we have emitted the byte containing the
		 * last piece of genuine data. There are three ways we can
		 * do this:
		 *
		 *  - Minimal flush. Output end-of-block and then open a
		 *    new static block. This takes 9 bits, which is
		 *    guaranteed to flush out the last genuine code in the
		 *    closed block; but allegedly zlib can't handle it.
		 *
		 *  - Zlib partial flush. Output EOB, open and close an
		 *    empty static block, and _then_ open the new block.
		 *    This is the best zlib can handle.
		 *
		 *  - Zlib sync flush. Output EOB, then an empty
		 *    _uncompressed_ block (000, then sync to byte
		 *    boundary, then send bytes 00 00 FF FF). Then open the
		 *    new block.
		 *
		 * For the moment, we will use Zlib partial flush.
		 */
		outbits(out, 0, 7);	       /* close block */
		outbits(out, 2, 3 + 7);	       /* empty static block */
		outbits(out, 2, 3);	       /* open new block */
    }

    out->comp_disabled = FALSE;

    *outblock = out->outbuf;
    *outlen = out->outlen;

    return 1;
}

/* ----------------------------------------------------------------------
 * Zlib decompression. Of course, even though our compressor always
 * uses static trees, our _decompressor_ has to be capable of
 * handling dynamic trees if it sees them.
 */

/*
 * Build a single-level decode table for elements
 * [minlength,maxlength) of the provided code/length tables, and
 * recurse to build subtables.
 */
struct CLZ77Lib::zlib_table *
                    CLZ77Lib::zlib_mkonetab(int *codes,
											unsigned char *lengths,
											int nsyms,
											int pfx, int pfxbits, int bits)
{
    struct CLZ77Lib::zlib_table *tab = (struct CLZ77Lib::zlib_table *)malloc(sizeof(struct CLZ77Lib::zlib_table));
    int pfxmask = (1 << pfxbits) - 1;
    int nbits, i, j, code;

    tab->table = (struct CLZ77Lib::zlib_tableentry*)malloc((1 << bits) * sizeof(struct CLZ77Lib::zlib_tableentry));
    tab->mask = (1 << bits) - 1;

    for (code = 0; code <= tab->mask; code++) {
	tab->table[code].code = -1;
	tab->table[code].nbits = 0;
	tab->table[code].nexttable = NULL;
    }

    for (i = 0; i < nsyms; i++) {
		if (lengths[i] <= pfxbits || (codes[i] & pfxmask) != pfx)
			continue;
		code = (codes[i] >> pfxbits) & tab->mask;
		for (j = code; j <= tab->mask; j += 1 << (lengths[i] - pfxbits)) {
			tab->table[j].code = i;
			nbits = lengths[i] - pfxbits;
			if (tab->table[j].nbits < nbits)
			tab->table[j].nbits = nbits;
		}
    }
    for (code = 0; code <= tab->mask; code++)
	{
		if (tab->table[code].nbits <= bits)
			continue;
		/* Generate a subtable. */
		tab->table[code].code = -1;
		nbits = tab->table[code].nbits - bits;
		if (nbits > 7)
			nbits = 7;
		tab->table[code].nbits = bits;
		tab->table[code].nexttable = zlib_mkonetab(codes, lengths, nsyms,
										   pfx | (code << pfxbits),
										   pfxbits + bits, nbits);
    }

    return tab;
}

/*
 * Build a decode table, given a set of Huffman tree lengths.
 */
struct CLZ77Lib::zlib_table *CLZ77Lib::zlib_mktable(unsigned char *lengths,int nlengths)
{
    int count[MAXCODELEN], startcode[MAXCODELEN], codes[MAXSYMS];
    int code, maxlen;
    int i, j;

    /* Count the codes of each length. */
    maxlen = 0;
    for (i = 1; i < MAXCODELEN; i++)
		count[i] = 0;

    for (i = 0; i < nlengths; i++) {
		count[lengths[i]]++;
		if (maxlen < lengths[i])
			maxlen = lengths[i];
    }
    /* Determine the starting code for each length block. */
    code = 0;
    for (i = 1; i < MAXCODELEN; i++) {
		startcode[i] = code;
		code += count[i];
		code <<= 1;
    }
    /* Determine the code for each symbol. Mirrored, of course. */
    for (i = 0; i < nlengths; i++) {
		code = startcode[lengths[i]]++;
		codes[i] = 0;
		for (j = 0; j < lengths[i]; j++) {
			codes[i] = (codes[i] << 1) | (code & 1);
			code >>= 1;
		}
    }

    /*
     * Now we have the complete list of Huffman codes. Build a
     * table.
     */
    return zlib_mkonetab(codes, lengths, nlengths, 0, 0, maxlen < 9 ? maxlen : 9);
}

int CLZ77Lib::zlib_freetable(struct CLZ77Lib::zlib_table **ztab)
{
    struct CLZ77Lib::zlib_table *tab;
    int code;
	
    if (ztab == NULL)
		return -1;
	
    if (*ztab == NULL)
		return 0;
	
    tab = *ztab;
	
    for (code = 0; code <= tab->mask; code++)
	{
		if (tab->table[code].nexttable != NULL)
			zlib_freetable(&tab->table[code].nexttable);
	}
		
	free(tab->table);
	tab->table = NULL;
	
	free(tab);
    *ztab = NULL;

    return (0);
}

void CLZ77Lib::zlib_decompress_init(void)
{
    unsigned char lengths[288];
	memset(&m_dctx,0,sizeof(m_dctx));//-- inside21_OSH 2003-05-22 --//

    memset(lengths, 8, 144);
    memset(lengths + 144, 9, 256 - 144);
    memset(lengths + 256, 7, 280 - 256);
    memset(lengths + 280, 8, 288 - 280);
    m_dctx.staticlentable = zlib_mktable(lengths, 288);
    memset(lengths, 5, 32);
    m_dctx.staticdisttable = zlib_mktable(lengths, 32);
    m_dctx.state = CLZ77Lib::zlib_decompress_ctx::START;/* even before header */
    m_dctx.currlentable = m_dctx.currdisttable = m_dctx.lenlentable = NULL;
    m_dctx.bits = 0;
    m_dctx.nbits = 0;
    //logevent("Initialised zlib (RFC1950) decompression\n");
}

int CLZ77Lib::zlib_huflookup(unsigned long *bitsp, int *nbitsp, struct zlib_table *tab)
{
	if( tab==NULL ) return -1;
    unsigned long bits = *bitsp;
    int nbits = *nbitsp;
    while (1)
	{
		struct CLZ77Lib::zlib_tableentry *ent;
		ent = &tab->table[bits & tab->mask];
		if (ent->nbits > nbits)
			return -1;		       /* not enough data */
		bits >>= ent->nbits;
		nbits -= ent->nbits;
		if (ent->code == -1)
			tab = ent->nexttable;
		else
		{
			*bitsp = bits;
			*nbitsp = nbits;
			return ent->code;
		}
    }
	return -1;
}

void CLZ77Lib::zlib_emit_char(int c)
{
    m_dctx.window[m_dctx.winpos] = c;
    m_dctx.winpos = (m_dctx.winpos + 1) & (WINSIZE - 1);
    if (m_dctx.outlen >= m_dctx.outsize)
	{
		m_dctx.outsize = m_dctx.outlen + 512;
		m_dctx.outblk = (unsigned char*)realloc(m_dctx.outblk, m_dctx.outsize);
    }
    m_dctx.outblk[m_dctx.outlen++] = c;
}

#define EATBITS(n) ( m_dctx.nbits -= (n), m_dctx.bits >>= (n) )

int CLZ77Lib::zlib_decompress_block(unsigned char *block, int len,
			  unsigned char **outblock, int *outlen)
{
    const CLZ77Lib::CODERECORD *rec;
    int code, blktype, rep, dist, nlen;
    static const unsigned char lenlenmap[] = {
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    m_dctx.outblk = NULL;
    m_dctx.outsize = m_dctx.outlen = 0;

    while (len > 0 || m_dctx.nbits > 0) 
	{
		while (m_dctx.nbits < 24 && len > 0)
		{
			m_dctx.bits |= (*block++) << m_dctx.nbits;
			m_dctx.nbits += 8;
			len--;
		}

		switch (m_dctx.state)
		{
		case CLZ77Lib::zlib_decompress_ctx::START:
			/* Expect 16-bit zlib header, which we'll dishonourably ignore. */
			if (m_dctx.nbits < 16)
				goto finished;	       /* done all we can */
			EATBITS(16);
			m_dctx.state = CLZ77Lib::zlib_decompress_ctx::OUTSIDEBLK;
			break;
		case CLZ77Lib::zlib_decompress_ctx::OUTSIDEBLK:
			/* Expect 3-bit block header. */
			if (m_dctx.nbits < 3)
				goto finished;	       /* done all we can */
			EATBITS(1);
			blktype = m_dctx.bits & 3;
			EATBITS(2);
			if (blktype == 0) {
				int to_eat = m_dctx.nbits & 7;
				m_dctx.state = CLZ77Lib::zlib_decompress_ctx::UNCOMP_LEN;
				EATBITS(to_eat);       /* align to byte boundary */
			} else if (blktype == 1) {
				m_dctx.currlentable = m_dctx.staticlentable;
				m_dctx.currdisttable = m_dctx.staticdisttable;
				m_dctx.state = CLZ77Lib::zlib_decompress_ctx::INBLK;
			} else if (blktype == 2) {
				m_dctx.state = CLZ77Lib::zlib_decompress_ctx::TREES_HDR;
			}
			break;
		case CLZ77Lib::zlib_decompress_ctx::TREES_HDR:
		/*
		* Dynamic block header. Five bits of HLIT, five of
		* HDIST, four of HCLEN.
			*/
			if (m_dctx.nbits < 5 + 5 + 4)
				goto finished;	       /* done all we can */
			m_dctx.hlit = 257 + (m_dctx.bits & 31);
			EATBITS(5);
			m_dctx.hdist = 1 + (m_dctx.bits & 31);
			EATBITS(5);
			m_dctx.hclen = 4 + (m_dctx.bits & 15);
			EATBITS(4);
			m_dctx.lenptr = 0;
			m_dctx.state = CLZ77Lib::zlib_decompress_ctx::TREES_LENLEN;
			memset(m_dctx.lenlen, 0, sizeof(m_dctx.lenlen));
			break;
		case CLZ77Lib::zlib_decompress_ctx::TREES_LENLEN:
			if (m_dctx.nbits < 3)
				goto finished;
			while (m_dctx.lenptr < m_dctx.hclen && m_dctx.nbits >= 3) {
				m_dctx.lenlen[lenlenmap[m_dctx.lenptr++]] =
					(unsigned char) (m_dctx.bits & 7);
				EATBITS(3);
			}
			if (m_dctx.lenptr == m_dctx.hclen) {
				m_dctx.lenlentable = zlib_mktable(m_dctx.lenlen, 19);
				m_dctx.state = CLZ77Lib::zlib_decompress_ctx::TREES_LEN;
				m_dctx.lenptr = 0;
			}
			break;
		case CLZ77Lib::zlib_decompress_ctx::TREES_LEN:
			if (m_dctx.lenptr >= m_dctx.hlit + m_dctx.hdist) {
				m_dctx.currlentable = zlib_mktable(m_dctx.lengths, m_dctx.hlit);
				m_dctx.currdisttable = zlib_mktable(m_dctx.lengths + m_dctx.hlit, m_dctx.hdist);
				zlib_freetable(&m_dctx.lenlentable);
				m_dctx.state = CLZ77Lib::zlib_decompress_ctx::INBLK;
				break;
			}
			code =	zlib_huflookup(&m_dctx.bits, &m_dctx.nbits, m_dctx.lenlentable);
			
			if (code == -1)
				goto finished;
			
			if (code < 16)
				m_dctx.lengths[m_dctx.lenptr++] = code;
			else {
				m_dctx.lenextrabits = (code == 16 ? 2 : code == 17 ? 3 : 7);
				m_dctx.lenaddon = (code == 18 ? 11 : 3);
				m_dctx.lenrep = (code == 16 && m_dctx.lenptr > 0 ?
					m_dctx.lengths[m_dctx.lenptr - 1] : 0);
				m_dctx.state = CLZ77Lib::zlib_decompress_ctx::TREES_LENREP;
			}
			break;
		case CLZ77Lib::zlib_decompress_ctx::TREES_LENREP:
			if (m_dctx.nbits < m_dctx.lenextrabits)
				goto finished;
			
			rep =m_dctx.lenaddon +(m_dctx.bits & ((1 << m_dctx.lenextrabits) - 1));
			
			EATBITS(m_dctx.lenextrabits);
			while (rep > 0 && m_dctx.lenptr < m_dctx.hlit + m_dctx.hdist) {
				m_dctx.lengths[m_dctx.lenptr] = m_dctx.lenrep;
				m_dctx.lenptr++;
				rep--;
			}
			m_dctx.state = CLZ77Lib::zlib_decompress_ctx::TREES_LEN;
			break;
		case CLZ77Lib::zlib_decompress_ctx::INBLK:
			code =	zlib_huflookup(&m_dctx.bits, &m_dctx.nbits, m_dctx.currlentable);
			if (code == -1)
				goto finished;
			
			if (code < 256)
				zlib_emit_char(code);
			else if (code == 256) {
				m_dctx.state = CLZ77Lib::zlib_decompress_ctx::OUTSIDEBLK;
				if (m_dctx.currlentable != m_dctx.staticlentable)
					zlib_freetable(&m_dctx.currlentable);
				if (m_dctx.currdisttable != m_dctx.staticdisttable)
					zlib_freetable(&m_dctx.currdisttable);
			} else if (code < 286) {   /* static tree can give >285; ignore */
				m_dctx.state = CLZ77Lib::zlib_decompress_ctx::GOTLENSYM;
				m_dctx.sym = code;
			}
			break;
		case CLZ77Lib::zlib_decompress_ctx::GOTLENSYM:
			rec = &lencodes[m_dctx.sym - 257];
			if (m_dctx.nbits < rec->extrabits)
				goto finished;
			
			m_dctx.len =rec->min + (m_dctx.bits & ((1 << rec->extrabits) - 1));
			
			EATBITS(rec->extrabits);
			m_dctx.state = CLZ77Lib::zlib_decompress_ctx::GOTLEN;
			break;
		case CLZ77Lib::zlib_decompress_ctx::GOTLEN:
			code = zlib_huflookup(&m_dctx.bits, &m_dctx.nbits, m_dctx.currdisttable);
			if (code == -1)
				goto finished;
			
			m_dctx.state = CLZ77Lib::zlib_decompress_ctx::GOTDISTSYM;
			m_dctx.sym = code;
			break;
		case CLZ77Lib::zlib_decompress_ctx::GOTDISTSYM:
			rec = &distcodes[m_dctx.sym];
			if (m_dctx.nbits < rec->extrabits)
				goto finished;
			dist = rec->min + (m_dctx.bits & ((1 << rec->extrabits) - 1));
			EATBITS(rec->extrabits);
			m_dctx.state = CLZ77Lib::zlib_decompress_ctx::INBLK;
			while (m_dctx.len--)
				zlib_emit_char(m_dctx.window[(m_dctx.winpos - dist) & (WINSIZE - 1)]);
			break;
		case CLZ77Lib::zlib_decompress_ctx::UNCOMP_LEN:
		/*
		* Uncompressed block. We expect to see a 16-bit LEN.
			*/
			if (m_dctx.nbits < 16)
				goto finished;
			m_dctx.uncomplen = m_dctx.bits & 0xFFFF;
			EATBITS(16);
			m_dctx.state = CLZ77Lib::zlib_decompress_ctx::UNCOMP_NLEN;
			break;
		case CLZ77Lib::zlib_decompress_ctx::UNCOMP_NLEN:
		/*
		* Uncompressed block. We expect to see a 16-bit NLEN,
		* which should be the one's complement of the previous
		* LEN.
			*/
			if (m_dctx.nbits < 16)
				goto finished;
			nlen = m_dctx.bits & 0xFFFF;
			EATBITS(16);
			m_dctx.state = CLZ77Lib::zlib_decompress_ctx::UNCOMP_DATA;
			break;
		case CLZ77Lib::zlib_decompress_ctx::UNCOMP_DATA:
			if (m_dctx.nbits < 8)
				goto finished;
			zlib_emit_char(m_dctx.bits & 0xFF);
			EATBITS(8);
			if (--m_dctx.uncomplen == 0)
				m_dctx.state = CLZ77Lib::zlib_decompress_ctx::OUTSIDEBLK;	/* end of uncompressed block */
			break;
		}
    }

finished:
    *outblock = m_dctx.outblk;
    *outlen = m_dctx.outlen;

    return 1;
}

CLZ77Lib::CLZ77Lib()
{
	m_bCompress_init = FALSE;
	m_bDecompress_init = FALSE;
	memset(&m_dctx,0,sizeof(m_dctx));//-- inside21_OSH 2003-05-22 --//
	memset(&m_ectx,0,sizeof(m_ectx));
}
CLZ77Lib::~CLZ77Lib()
{
	if(m_ectx.ictx) free(m_ectx.ictx);
	if(m_ectx.userdata) free(m_ectx.userdata);

	if(m_dctx.staticlentable)
	{
		if(m_dctx.currlentable == m_dctx.staticlentable)
			m_dctx.currlentable = NULL;

		zlib_freetable(&m_dctx.staticlentable);
	}

	if(m_dctx.staticdisttable)
	{
		if(m_dctx.currdisttable == m_dctx.staticdisttable)
			m_dctx.currdisttable=NULL;

		zlib_freetable(&m_dctx.staticdisttable);
	}

	if(m_dctx.lenlentable)     zlib_freetable(&m_dctx.lenlentable);
	if(m_dctx.currlentable)    zlib_freetable(&m_dctx.currlentable);
	if(m_dctx.currdisttable)   zlib_freetable(&m_dctx.currdisttable);

}

BOOL CLZ77Lib::Compress(unsigned char *block, int len, unsigned char **outblock, int *outlen)
{
	// Log("LZ77>> %d\n", outlen!=NULL );
	// Log("LZ77>> %d\n", outblock!=NULL );

	if(0 >= len)
	{
		if(outblock) *outblock=NULL;
		if(outlen) *outlen=0;
		return FALSE;
	}

	if(FALSE == m_bCompress_init)
	{
		memset(&m_dctx, 0, sizeof(m_dctx));
		zlib_compress_init();
		m_bCompress_init = TRUE;
	}

	if(outblock) *outblock=NULL;
	if(outlen) *outlen=0;

	if(1==zlib_compress_block(block, len,	outblock, outlen))
		return TRUE;

	return FALSE;
}

BOOL CLZ77Lib::Decompress(unsigned char *block, int len, unsigned char **outblock, int *outlen)
{
	// Log("LZ77>> %d\n", outlen!=NULL );
	// Log("LZ77>> %d\n", outblock!=NULL );

	if(FALSE == m_bDecompress_init)
	{
		zlib_decompress_init();
		m_bDecompress_init = TRUE;
	}

	if(outblock) *outblock=NULL;
	if(outlen) *outlen=0;

	if(1 == zlib_decompress_block(block, len, outblock, outlen))
		return TRUE;

	return FALSE;
}

/*
 * CRC32 implementation.
 *
 * The basic concept of a CRC is that you treat your bit-string
 * abcdefg... as a ludicrously long polynomial M=a+bx+cx^2+dx^3+...
 * over Z[2]. You then take a modulus polynomial P, and compute the
 * remainder of M on division by P. Thus, an erroneous message N
 * will only have the same CRC if the difference E = M-N is an
 * exact multiple of P. (Note that as we are working over Z[2], M-N
 * = N-M = M+N; but that's not very important.)
 *
 * What makes the CRC good is choosing P to have good properties:
 *
 *  - If its first and last terms are both nonzero then it cannot
 *    be a factor of any single term x^i. Therefore if M and N
 *    differ by exactly one bit their CRCs will guaranteeably
 *    be distinct.
 *
 *  - If it has a prime (irreducible) factor with three terms then
 *    it cannot divide a polynomial of the form x^i(1+x^j).
 *    Therefore if M and N differ by exactly _two_ bits they will
 *    have different CRCs.
 *
 *  - If it has a factor (x+1) then it cannot divide a polynomial
 *    with an odd number of terms. Therefore if M and N differ by
 *    _any odd_ number of bits they will have different CRCs.
 *
 *  - If the error term E is of the form x^i*B(x) where B(x) has
 *    order less than P (i.e. a short _burst_ of errors) then P
 *    cannot divide E (since no polynomial can divide a shorter
 *    one), so any such error burst will be spotted.
 *
 * The CRC32 standard polynomial is
 *   x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
 *
 * In fact, we don't compute M mod P; we compute M*x^32 mod P.
 *
 * The concrete implementation of the CRC is this: we maintain at
 * all times a 32-bit word which is the current remainder of the
 * polynomial mod P. Whenever we receive an extra bit, we multiply
 * the existing remainder by x, add (XOR) the x^32 term thus
 * generated to the new x^32 term caused by the incoming bit, and
 * remove the resulting combined x^32 term if present by replacing
 * it with (P-x^32).
 *
 * Bit 0 of the word is the x^31 term and bit 31 is the x^0 term.
 * Thus, multiplying by x means shifting right. So the actual
 * algorithm goes like this:
 *
 *   x32term = (crcword & 1) ^ newbit;
 *   crcword = (crcword >> 1) ^ (x32term * 0xEDB88320);
 *
 * In practice, we pre-compute what will happen to crcword on any
 * given sequence of eight incoming bits, and store that in a table
 * which we then use at run-time to do the job:
 * 
 *   outgoingplusnew = (crcword & 0xFF) ^ newbyte;
 *   crcword = (crcword >> 8) ^ table[outgoingplusnew];
 *
 * where table[outgoingplusnew] is computed by setting crcword=0
 * and then iterating the first code fragment eight times (taking
 * the incoming byte low bit first).
 *
 * Note that all shifts are rightward and thus no assumption is
 * made about exact word length! (Although word length must be at
 * _least_ 32 bits, but ANSI C guarantees this for `unsigned long'
 * anyway.)
 */


/* ----------------------------------------------------------------------
 * Multi-function module. Can be compiled three ways.
 *
 *  - Compile with no special #defines. Will generate a table
 *    that's already initialised at compile time, and one function
 *    crc32(buf,len) that uses it. Normal usage.
 *
 *  - Compile with INITFUNC defined. Will generate an uninitialised
 *    array as the table, and as well as crc32(buf,len) it will
 *    also generate void crc32_init(void) which sets up the table
 *    at run time. Useful if binary size is important.
 *
 *  - Compile with GENPROGRAM defined. Will create a standalone
 *    program that does the initialisation and outputs the table as
 *    C code.
 */

#define POLY (0xEDB88320L)

/*
 * This variant of the code has the data already prepared.
 */
const unsigned long CLZ77Lib::CRC32_TABLE[256] = {
    0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL,
    0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
    0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L,
    0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
    0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
    0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
    0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL,
    0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
    0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L,
    0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
    0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
    0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
    0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L,
    0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
    0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
    0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
    0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL,
    0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
    0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L,
    0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
    0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL,
    0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
    0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL,
    0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
    0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
    0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
    0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L,
    0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
    0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L,
    0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
    0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L,
    0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
    0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
    0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
    0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
    0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
    0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL,
    0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
    0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL,
    0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
    0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L,
    0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
    0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L,
    0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
    0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
    0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
    0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L,
    0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
    0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL,
    0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
    0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L,
    0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
    0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL,
    0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
    0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
    0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
    0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L,
    0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
    0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L,
    0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
    0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L,
    0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
    0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L,
    0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};

unsigned long CLZ77Lib::CRC32(const void *buf, size_t len)
{
	unsigned long crcword=0;
    const unsigned char *p = (const unsigned char *) buf;
    while (len--) {
	unsigned long newbyte = *p++;
	newbyte ^= crcword & 0xFFL;
	crcword = (crcword >> 8) ^ CRC32_TABLE[newbyte];
    }
    return crcword;
    
}
/*
 *
 *                          BASE64  (Binary -> Text)
 *
 */

int CLZ77Lib::base64_decode_atom(unsigned char *atom, unsigned char *out)
{
    int vals[4];
    int i, v, len;
    unsigned word;
    char c;

    for (i = 0; i < 4; i++) {
	c = atom[i];
	if (c >= 'A' && c <= 'Z')
	    v = c - 'A';
	else if (c >= 'a' && c <= 'z')
	    v = c - 'a' + 26;
	else if (c >= '0' && c <= '9')
	    v = c - '0' + 52;
	else if (c == '+')
	    v = 62;
	else if (c == '/')
	    v = 63;
	else if (c == '=')
	    v = -1;
	else
	    return 0;		       /* invalid atom */
	vals[i] = v;
    }

    if (vals[0] == -1 || vals[1] == -1)
	return 0;
    if (vals[2] == -1 && vals[3] != -1)
	return 0;

    if (vals[3] != -1)
	len = 3;
    else if (vals[2] != -1)
	len = 2;
    else
	len = 1;

    word = ((vals[0] << 18) |
	    (vals[1] << 12) | ((vals[2] & 0x3F) << 6) | (vals[3] & 0x3F));
    out[0] = (word >> 16) & 0xFF;
    if (len > 1)
	out[1] = (word >> 8) & 0xFF;
    if (len > 2)
	out[2] = word & 0xFF;
    return len;
}

void CLZ77Lib::base64_encode_atom(unsigned char *data, int n, char *out)
{
    static const char base64_chars[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned word;

    word = data[0] << 16;
    
    if (n > 1)
	    word |= data[1] << 8;

    if (n > 2)
	    word |= data[2];
	    
    out[0] = base64_chars[(word >> 18) & 0x3F];
    out[1] = base64_chars[(word >> 12) & 0x3F];
    
    if (n > 1)
	    out[2] = base64_chars[(word >> 6) & 0x3F];
    else
	    out[2] = '=';
	    
    if (n > 2)
	    out[3] = base64_chars[word & 0x3F];
    else
    	out[3] = '=';
}

BOOL CLZ77Lib::BASE64_Encode(unsigned char *data, int datalen, unsigned char **out, int *outlen)
{
	// Log("LZ77>> %d\n", outlen!=NULL );
	// Log("LZ77>> %d\n", out!=NULL );

    int encodelen = 0;
    char _out[4];
    int n, i;
    unsigned char * szEncode;
	
	if(0>= datalen) return FALSE;

    /* n = datalen * (8bit/6bit) */
    n = datalen + (datalen >> 1);
    szEncode = (unsigned char *)malloc(n);
    if(NULL == szEncode) return FALSE;
    
    while (datalen > 0)
    {
	    n = (datalen < 3 ? datalen : 3);
	    base64_encode_atom(data, n, _out);
	    data += n;
	    datalen -= n;
	    for (i = 0; i < 4; i++)
        {
	    	szEncode[encodelen] = _out[i];
	    	encodelen++;
		}
    }// while
    
    *out    = szEncode;
    *outlen = encodelen;
    
    return TRUE;
}

BOOL CLZ77Lib::BASE64_Decode(unsigned char *data, int datalen, unsigned char **out, int *outlen)
{
	// Log("LZ77>> %d\n", outlen!=NULL );
	// Log("LZ77>> %d\n", out!=NULL );

    unsigned char *szDecode;
    int nDecodeLen;
    int i,  k;

    szDecode = (unsigned char *)malloc(datalen);
    if(NULL == szDecode) return FALSE;

    nDecodeLen = 0;
    for (i = 0; i < datalen; i+=4 )
    {
		k = base64_decode_atom(data + i, szDecode + nDecodeLen);
		if (!k) {

			free(szDecode);
			return FALSE;
		}
		nDecodeLen += k;

    }
    *outlen = nDecodeLen;
	*out    = szDecode;

    return TRUE;
}
