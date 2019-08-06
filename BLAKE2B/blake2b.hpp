#include <iostream>
#include <assert.h>
# define BLAKE2B_SALTBYTES     16
# define BLAKE2B_PERSONALBYTES 16
# define BLAKE2B_BLOCKBYTES    128
# define BLAKE2B_KEYBYTES	   64



struct blake2b_param_st {
	uint8_t  digest_length; /* 1 */
	uint8_t  key_length;    /* 2 */
	uint8_t  fanout;        /* 3 */
	uint8_t  depth;         /* 4 */
	uint8_t  leaf_length[4];/* 8 */
	uint8_t  node_offset[8];/* 16 */
	uint8_t  node_depth;    /* 17 */
	uint8_t  inner_length;  /* 18 */
	uint8_t  reserved[14];  /* 32 */
	uint8_t  salt[BLAKE2B_SALTBYTES]; /* 48 */
	uint8_t  personal[BLAKE2B_PERSONALBYTES];  /* 64 */
};
struct blake2b_ctx_st {
	uint64_t h[8];
	uint64_t t[2];
	uint64_t f[2];
	uint8_t  buf[BLAKE2B_BLOCKBYTES];
	size_t   buflen;
	size_t   outlen;
};
typedef struct blake2b_ctx_st BLAKE2B_CTX;
typedef struct blake2b_param_st BLAKE2B_PARAM;
struct evp_mac_impl_st {
	BLAKE2B_CTX ctx;
	BLAKE2B_PARAM params;
	unsigned char key[BLAKE2B_KEYBYTES];
};


typedef struct evp_mac_impl_st EVP_MAC_IMPL;


int blake2b_update(BLAKE2B_CTX* c, const unsigned char* data, size_t datalen);
int blake2b_final(unsigned char* md, BLAKE2B_CTX* c);
int blake2b_init_key(BLAKE2B_CTX* c, const BLAKE2B_PARAM* P, const void* key);


static int blake2b_mac_init(EVP_MAC_IMPL* macctx);
static int blake2b_mac_update(EVP_MAC_IMPL* macctx, const unsigned char* data, size_t datalen);
static int blake2b_mac_final(EVP_MAC_IMPL* macctx, unsigned char* out);


int blake2b_hash(const unsigned char* message, size_t length, unsigned char* out, unsigned int BLAKE2B_DIGEST_LENGTH);
int blake2b_mac_hash(const unsigned char* message, size_t length, unsigned char* out, unsigned char* key, unsigned int key_len, unsigned int BLAKE2B_DIGEST_LENGTH);
