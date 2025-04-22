#ifndef __Z77LIB_V1_1_H_
#define __Z77LIB_V1_1_H_

/* ----------------------------------------------------------------------
 * Basic LZ77 code. This bit is designed modularly, so it could be
 * ripped out and used in a different LZ77 compressor. Go to it,
 * and good luck :-)


#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#ifndef BOOL
#define BOOL int
#endif 

 */

#include "../util/def.h"


typedef unsigned int word32;
typedef unsigned int uint32;

/*
 * Modifiable parameters.
 */
#define WINSIZE 32768		       /* window size. Must be power of 2! */
#define HASHMAX 2039		       /* one more than max hash value */
#define MAXMATCH 32		       /* how many matches we track */
#define HASHCHARS 3		       /* how many chars make a hash */

#define INVALID -1		       /* invalid hash _and_ invalid offset */

#define MAXCODELEN 16
#define MAXSYMS 288

class CLZ77Lib
{
protected:
	typedef struct
	{
		short code, extrabits;
		int min, max;
	} CODERECORD;

	/*
	 * This compressor takes a less slapdash approach than the
	 * gzip/zlib one. Rather than allowing our hash chains to fall into
	 * disuse near the far end, we keep them doubly linked so we can
	 * _find_ the far end, and then every time we add a new byte to the
	 * window (thus rolling round by one and removing the previous
	 * byte), we can carefully remove the hash chain entry.
	 */

	struct WindowEntry
	{
		short next, prev;		       /* array indices within the window */
		short hashval;
	};

	struct HashEntry
	{
		short first;		       /* window index of first in chain */
	};

	struct LZ77InternalContext
	{
		struct WindowEntry win[WINSIZE];
		unsigned char data[WINSIZE];
		int winpos;
		struct HashEntry hashtab[HASHMAX];
		unsigned char pending[HASHCHARS];
		int npending;
	};


	struct LZ77Context
	{
	    struct LZ77InternalContext *ictx;
		void *userdata;
		void (*literal) (struct LZ77Context * ctx, unsigned char c);
		void (*match) (struct LZ77Context * ctx, int distance, int len);
	};

	struct LZ77Context m_ectx;

	struct Match
	{
		int distance, len;
	};

	struct Outbuf
	{
		unsigned char *outbuf;
		int outlen, outsize;
		unsigned long outbits;
		int noutbits;
		int firstblock;
		int comp_disabled;
	};

	/*
	 * Initialise the private fields of an LZ77Context. It's up to the
	 * user to initialise the public fields.
	 */
	int lz77_init(struct LZ77Context *ctx);

	/*
	 * Supply data to be compressed. Will update the private fields of
	 * the LZ77Context, and will call literal() and match() to output.
	 * If `compress' is FALSE, it will never emit a match, but will
	 * instead call literal() for everything.
	 */
	void  lz77_compress(struct LZ77Context *ctx, unsigned char *data, int len, int compress);

	int   lz77_hash(unsigned char *data);

	void  lz77_advance(struct LZ77InternalContext *st, unsigned char c, int hash);

	void  outbits(struct Outbuf *out, unsigned long bits, int nbits);
	static const unsigned char mirrorbytes[256];
	static const CODERECORD lencodes[];
	static const CODERECORD distcodes[];
	void  zlib_literal(struct LZ77Context *ectx, unsigned char c);
	void  zlib_match(struct LZ77Context *ectx, int distance, int len);
	int   zlib_compress_block(unsigned char *block, int len, unsigned char **outblock, int *outlen);

protected:
	/*
	 * The way we work the Huffman decode is to have a table lookup on
	 * the first N bits of the input stream (in the order they arrive,
	 * of course, i.e. the first bit of the Huffman code is in bit 0).
	 * Each table entry lists the number of bits to consume, plus
	 * either an output code or a pointer to a secondary table.
	 */
	struct zlib_tableentry;
	struct zlib_table {
		int mask;			       /* mask applied to input bit stream */
		struct zlib_tableentry *table;
	};

	struct zlib_tableentry {
		unsigned char nbits;
		short code;
		struct zlib_table *nexttable;
	};

	struct zlib_decompress_ctx
	{
		struct zlib_table *staticlentable, *staticdisttable;
		struct zlib_table *currlentable, *currdisttable, *lenlentable;
		enum {
			START, OUTSIDEBLK,
			TREES_HDR, TREES_LENLEN, TREES_LEN, TREES_LENREP,
			INBLK, GOTLENSYM, GOTLEN, GOTDISTSYM,
			UNCOMP_LEN, UNCOMP_NLEN, UNCOMP_DATA
		} state;
		int sym, hlit, hdist, hclen, lenptr, lenextrabits, lenaddon, len,
		lenrep;
		int uncomplen;
		unsigned char lenlen[19];
		unsigned char lengths[286 + 32];
		unsigned long bits;
		int nbits;
		unsigned char window[WINSIZE];
		int winpos;
		unsigned char *outblk;
		int outlen, outsize;
	};

	struct zlib_table *zlib_mkonetab(int *codes, unsigned char *lengths,
					int nsyms, int pfx, int pfxbits, int bits);
	struct zlib_table *zlib_mktable(unsigned char *lengths,int nlengths);
	int zlib_freetable(struct zlib_table **ztab);

	int  zlib_huflookup(unsigned long *bitsp, int *nbitsp, struct zlib_table *tab);
	void zlib_emit_char(int c);
	int  zlib_decompress_block(unsigned char *block, int len, unsigned char **outblock, int *outlen);

	void zlib_compress_init(void);
	void zlib_decompress_init(void);

	struct zlib_decompress_ctx m_dctx;

	BOOL m_bCompress_init;
	BOOL m_bDecompress_init;

	static const unsigned long CRC32_TABLE[256];
	static int base64_decode_atom(unsigned char *atom, unsigned char *out);
	static void base64_encode_atom(unsigned char *data, int n, char *out);
public:
	CLZ77Lib();
	virtual ~CLZ77Lib();
	BOOL Disable_Compression(void);
	BOOL Compress(unsigned char *block, int len, unsigned char **outblock, int *outlen);
	BOOL Decompress(unsigned char *block, int len, unsigned char **outblock, int *outlen);
public:
	static	unsigned long CRC32(const void *buf, size_t len);
	static int BASE64_Decode(unsigned char *data, int datalen, unsigned char **out, int *outlen);
	static int BASE64_Encode(unsigned char *data, int datalen, unsigned char **out, int *outlen);
 
};

#endif



