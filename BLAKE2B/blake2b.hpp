#pragma once
#include <cstdint>
#include <iostream>
#include <cassert>


#define BLAKE2B_BLOCKBYTES    128
#define BLAKE2B_OUTBYTES      64
#define BLAKE2B_KEYBYTES      64
#define BLAKE2B_SALTBYTES     16
#define BLAKE2B_PERSONALBYTES 16
#define BLAKE2B_DIGEST_LENGTH 64

struct blake2b_ctx_st {
	uint64_t h[8];
	uint64_t t[2];
	uint64_t f[2];
	uint8_t  buf[BLAKE2B_BLOCKBYTES];
	size_t   buflen;
};
typedef struct blake2b_ctx_st BLAKE2B_CTX;
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
typedef struct blake2b_param_st BLAKE2B_PARAM;





int BLAKE2b_Init(BLAKE2B_CTX *c, uint8_t digest_len = 64);
int BLAKE2b_Update(BLAKE2B_CTX *c, const unsigned char *data, size_t datalen);
int BLAKE2b_Final(unsigned char *md, BLAKE2B_CTX *c);
bool BLAKE2b_hash(const unsigned char* data, unsigned int data_len, const unsigned char* out, uint8_t digest_len);

