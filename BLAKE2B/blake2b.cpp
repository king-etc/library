
#include "blake2b.hpp"

static const uint64_t blake2b_IV[8] =
{
	0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
	0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
	0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
	0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};


static const uint8_t blake2b_sigma[12][16] =
{
	{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } ,
	{ 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 } ,
	{ 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 } ,
	{  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 } ,
	{  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 } ,
	{  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 } ,
	{ 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 } ,
	{ 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 } ,
	{  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 } ,
	{ 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0 } ,
	{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } ,
	{ 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 }
};

static  void store32(uint8_t* dst, uint32_t w)
{
	const union {
		long one;
		char little;
	} is_endian = { 1 };

	if (is_endian.little) {
		memcpy(dst, &w, sizeof(w));
	}
	else {
		uint8_t* p = (uint8_t*)dst;
		int i;

		for (i = 0; i < 4; i++)
			p[i] = (uint8_t)(w >> (8 * i));
	}
}

static  void store64(uint8_t* dst, uint64_t w)
{
	const union {
		long one;
		char little;
	} is_endian = { 1 };

	if (is_endian.little) {
		memcpy(dst, &w, sizeof(w));
	}
	else {
		uint8_t* p = (uint8_t*)dst;
		int i;

		for (i = 0; i < 8; i++)
			p[i] = (uint8_t)(w >> (8 * i));
	}
}

static uint64_t load64(const uint8_t* src)
{
	const union {
		long one;
		char little;
	} is_endian = { 1 };

	if (is_endian.little) {
		uint64_t w;
		memcpy(&w, src, sizeof(w));
		return w;
	}
	else {
		uint64_t w = ((uint64_t)src[0])
			| ((uint64_t)src[1] << 8)
			| ((uint64_t)src[2] << 16)
			| ((uint64_t)src[3] << 24)
			| ((uint64_t)src[4] << 32)
			| ((uint64_t)src[5] << 40)
			| ((uint64_t)src[6] << 48)
			| ((uint64_t)src[7] << 56);
		return w;
	}
}
static uint64_t rotr64(const uint64_t w, const unsigned int c)
{
	return (w >> c) | (w << (64 - c));
}

static  void blake2b_init0(BLAKE2B_CTX* S)
{
	int i;

	memset(S, 0, sizeof(BLAKE2B_CTX));
	for (i = 0; i < 8; ++i) {
		S->h[i] = blake2b_IV[i];
	}
}


void blake2b_param_init(BLAKE2B_PARAM* P, unsigned int BLAKE2B_DIGEST_LENGTH)
{
	P->digest_length = BLAKE2B_DIGEST_LENGTH;
	P->key_length = 0;
	P->fanout = 1;
	P->depth = 1;
	store32(P->leaf_length, 0);
	store64(P->node_offset, 0);
	P->node_depth = 0;
	P->inner_length = 0;
	memset(P->reserved, 0, sizeof(P->reserved));
	memset(P->salt, 0, sizeof(P->salt));
	memset(P->personal, 0, sizeof(P->personal));
}

static void blake2b_init_param(BLAKE2B_CTX* S, const BLAKE2B_PARAM* P)
{
	size_t i;
	const uint8_t* p = (const uint8_t*)(P);

	blake2b_init0(S);
	S->outlen = P->digest_length;

	/* The param struct is carefully hand packed, and should be 64 bytes on
	 * every platform. */
	assert(sizeof(BLAKE2B_PARAM) == 64);
	/* IV XOR ParamBlock */
	for (i = 0; i < 8; ++i) {
		S->h[i] ^= load64(p + sizeof(S->h[i]) * i);
	}
}

static void blake2b_compress(BLAKE2B_CTX* S,
	const uint8_t* blocks,
	size_t len)
{
	uint64_t m[16];
	uint64_t v[16];
	int i;
	size_t increment;

	/*
	 * There are two distinct usage vectors for this function:
	 *
	 * a) BLAKE2b_Update uses it to process complete blocks,
	 *    possibly more than one at a time;
	 *
	 * b) BLAK2b_Final uses it to process last block, always
	 *    single but possibly incomplete, in which case caller
	 *    pads input with zeros.
	 */
	assert(len < BLAKE2B_BLOCKBYTES || len % BLAKE2B_BLOCKBYTES == 0);

	/*
	 * Since last block is always processed with separate call,
	 * |len| not being multiple of complete blocks can be observed
	 * only with |len| being less than BLAKE2B_BLOCKBYTES ("less"
	 * including even zero), which is why following assignment doesn't
	 * have to reside inside the main loop below.
	 */
	increment = len < BLAKE2B_BLOCKBYTES ? len : BLAKE2B_BLOCKBYTES;

	for (i = 0; i < 8; ++i) {
		v[i] = S->h[i];
	}

	do {
		for (i = 0; i < 16; ++i) {
			m[i] = load64(blocks + i * sizeof(m[i]));
		}

		/* blake2b_increment_counter */
		S->t[0] += increment;
		S->t[1] += (S->t[0] < increment);

		v[8] = blake2b_IV[0];
		v[9] = blake2b_IV[1];
		v[10] = blake2b_IV[2];
		v[11] = blake2b_IV[3];
		v[12] = S->t[0] ^ blake2b_IV[4];
		v[13] = S->t[1] ^ blake2b_IV[5];
		v[14] = S->f[0] ^ blake2b_IV[6];
		v[15] = S->f[1] ^ blake2b_IV[7];
#define G(r,i,a,b,c,d) \
        do { \
            a = a + b + m[blake2b_sigma[r][2*i+0]]; \
            d = rotr64(d ^ a, 32); \
            c = c + d; \
            b = rotr64(b ^ c, 24); \
            a = a + b + m[blake2b_sigma[r][2*i+1]]; \
            d = rotr64(d ^ a, 16); \
            c = c + d; \
            b = rotr64(b ^ c, 63); \
        } while (0)
#define ROUND(r)  \
        do { \
            G(r,0,v[ 0],v[ 4],v[ 8],v[12]); \
            G(r,1,v[ 1],v[ 5],v[ 9],v[13]); \
            G(r,2,v[ 2],v[ 6],v[10],v[14]); \
            G(r,3,v[ 3],v[ 7],v[11],v[15]); \
            G(r,4,v[ 0],v[ 5],v[10],v[15]); \
            G(r,5,v[ 1],v[ 6],v[11],v[12]); \
            G(r,6,v[ 2],v[ 7],v[ 8],v[13]); \
            G(r,7,v[ 3],v[ 4],v[ 9],v[14]); \
        } while (0)
#if defined(OPENSSL_SMALL_FOOTPRINT)
		/* 3x size reduction on x86_64, almost 7x on ARMv8, 9x on ARMv4 */
		for (i = 0; i < 12; i++) {
			ROUND(i);
		}
#else
		ROUND(0);
		ROUND(1);
		ROUND(2);
		ROUND(3);
		ROUND(4);
		ROUND(5);
		ROUND(6);
		ROUND(7);
		ROUND(8);
		ROUND(9);
		ROUND(10);
		ROUND(11);
#endif

		for (i = 0; i < 8; ++i) {
			S->h[i] = v[i] ^= v[i + 8] ^ S->h[i];
		}
#undef G
#undef ROUND
		blocks += increment;
		len -= increment;
	} while (len);
}
int blake2b_update(BLAKE2B_CTX* c, const unsigned char* data, size_t datalen)
{
	const uint8_t* in = data;
	size_t fill;

	/*
	 * Intuitively one would expect intermediate buffer, c->buf, to
	 * store incomplete blocks. But in this case we are interested to
	 * temporarily stash even complete blocks, because last one in the
	 * stream has to be treated in special way, and at this point we
	 * don't know if last block in *this* call is last one "ever". This
	 * is the reason for why |datalen| is compared as >, and not >=.
	 */
	fill = sizeof(c->buf) - c->buflen;
	if (datalen > fill) {
		if (c->buflen) {
			memcpy(c->buf + c->buflen, in, fill); /* Fill buffer */
			blake2b_compress(c, c->buf, BLAKE2B_BLOCKBYTES);
			c->buflen = 0;
			in += fill;
			datalen -= fill;
		}
		if (datalen > BLAKE2B_BLOCKBYTES) {
			size_t stashlen = datalen % BLAKE2B_BLOCKBYTES;
			/*
			 * If |datalen| is a multiple of the blocksize, stash
			 * last complete block, it can be final one...
			 */
			stashlen = stashlen ? stashlen : BLAKE2B_BLOCKBYTES;
			datalen -= stashlen;
			blake2b_compress(c, in, datalen);
			in += datalen;
			datalen = stashlen;
		}
	}

	assert(datalen <= BLAKE2B_BLOCKBYTES);

	memcpy(c->buf + c->buflen, in, datalen);
	c->buflen += datalen; /* Be lazy, do not compress */

	return 1;
}
static void blake2b_set_lastblock(BLAKE2B_CTX* S)
{
	S->f[0] = -1;
}
#define BLAKE2B_OUTBYTES 32
int blake2b_final(unsigned char* md, BLAKE2B_CTX * c)
{
	uint8_t outbuffer[BLAKE2B_OUTBYTES] = { 0 };
	uint8_t* target = outbuffer;
	int iter = (c->outlen + 7) / 8;
	int i;

	/* Avoid writing to the temporary buffer if possible */
	if ((c->outlen % sizeof(c->h[0])) == 0)
		target = md;

	blake2b_set_lastblock(c);
	/* Padding */
	memset(c->buf + c->buflen, 0, sizeof(c->buf) - c->buflen);
	blake2b_compress(c, c->buf, c->buflen);

	/* Output full hash to buffer */
	for (i = 0; i < iter; ++i)
		store64(target + sizeof(c->h[i]) * i, c->h[i]);

	if (target != md)
		memcpy(md, target, c->outlen);

	memset(c, 0, sizeof(BLAKE2B_CTX));
	return 1;
}

int blake2b_init_key(BLAKE2B_CTX* c, const BLAKE2B_PARAM* P, const void* key)
{
	blake2b_init_param(c, P);

	/* Pad the key to form first data block */
	{
		uint8_t block[BLAKE2B_BLOCKBYTES] = { 0 };

		memcpy(block, key, P->key_length);
		blake2b_update(c, block, BLAKE2B_BLOCKBYTES);
		memset(block, 0, BLAKE2B_BLOCKBYTES);
	}

	return 1;
}

static int blake2b_mac_init(EVP_MAC_IMPL* macctx)
{
	/* Check key has been set */
	if (macctx->params.key_length == 0) {
		return 0;
	}

	return blake2b_init_key(&macctx->ctx, &macctx->params, macctx->key);
}

static int blake2b_mac_update(EVP_MAC_IMPL* macctx, const unsigned char* data,
	size_t datalen)
{
	return blake2b_update(&macctx->ctx, data, datalen);
}

static int blake2b_mac_final(EVP_MAC_IMPL* macctx, unsigned char* out)
{
	return blake2b_final(out, &macctx->ctx);
}

int blake2b_hash(const unsigned char* message, size_t length, unsigned char* out, unsigned int BLAKE2B_DIGEST_LENGTH)
{
	BLAKE2B_PARAM P;
	BLAKE2B_CTX temp;
	blake2b_param_init(&P, BLAKE2B_DIGEST_LENGTH);
	blake2b_init_param(&temp, &P);
	auto x1 = blake2b_update(&temp, message, length);
	auto x2 = blake2b_final(out, &temp);
	return x1 && x2;
}
int blake2b_mac_hash(const unsigned char* message, size_t length, unsigned char* out, unsigned char* key, unsigned int key_len, unsigned int BLAKE2B_DIGEST_LENGTH)
{
	EVP_MAC_IMPL temp;
	memcpy(temp.key, key, key_len);
	blake2b_param_init(&temp.params, BLAKE2B_DIGEST_LENGTH);
	temp.params.key_length = key_len;
	bool x = true;
	x = x && blake2b_mac_init(&temp);
	x = x && blake2b_mac_update(&temp, message, length);
	x = x && blake2b_mac_final(&temp, out);
	return x;
}
