#include <iostream>
#include <cstdint>





# define SHA512_DIGEST_LENGTH    64
#  define U64(C)     C##UI64
#  define SHA_LONG64 unsigned long
# define SHA512_CBLOCK   (SHA_LBLOCK*8)
# define SHA_LBLOCK      16


static const int64_t kBottom25Bits = 0x1ffffffLL;
static const int64_t kBottom26Bits = 0x3ffffffLL;
static const int64_t kTop39Bits = 0xfffffffffe000000LL;
static const int64_t kTop38Bits = 0xfffffffffc000000LL;



typedef struct SHA512state_st {
	SHA_LONG64 h[8];
	SHA_LONG64 Nl, Nh;
	union {
		SHA_LONG64 d[SHA_LBLOCK];
		unsigned char p[SHA512_CBLOCK];
	} u;
	unsigned int num, md_len;
} SHA512_CTX;








typedef int32_t fe[10];
typedef struct {
	fe X;
	fe Y;
	fe Z;
	fe T;
} ge_p3;
typedef struct {
	fe X;
	fe Y;
	fe Z;
} ge_p2;
typedef struct {
	fe X;
	fe Y;
	fe Z;
	fe T;
} ge_p1p1;

typedef struct {
	fe yplusx;
	fe yminusx;
	fe xy2d;
} ge_precomp;

typedef struct {
	fe YplusX;
	fe YminusX;
	fe Z;
	fe T2d;
} ge_cached;





void ED25519_public_from_private(uint8_t out_public_key[32],
	const uint8_t private_key[32]);
int ED25519_sign(uint8_t *out_sig, const uint8_t *message, size_t message_len,
	const uint8_t public_key[32], const uint8_t private_key[32]);
void X25519_public_from_private(uint8_t out_public_value[32],
	const uint8_t private_key[32]);
int ED25519_verify(const uint8_t *message, size_t message_len,
	const uint8_t signature[64], const uint8_t public_key[32]);
int X25519(uint8_t out_shared_key[32], const uint8_t private_key[32],
	const uint8_t peer_public_value[32]);
