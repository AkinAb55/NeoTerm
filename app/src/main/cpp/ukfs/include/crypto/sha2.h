#ifndef _UK_CRYPTO_SHA2_H
#define _UK_CRYPTO_SHA2_H
#include <linux/types.h>
#define SHA256_DIGEST_SIZE 32
#define SHA256_BLOCK_SIZE  64
struct sha256_state { u32 state[8]; u64 count; u8 buf[64]; };
void sha256(const u8 *data, unsigned int len, u8 *out);
#endif
