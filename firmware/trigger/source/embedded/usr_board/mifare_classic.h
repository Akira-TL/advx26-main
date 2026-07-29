#ifndef MIFARE_CLASSIC_H
#define MIFARE_CLASSIC_H

#include "pn532_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M1_BLOCK_LEN    16
#define M1_1K_BLOCKS    64   /* 16 sectors x 4 blocks */
#define M1_4K_BLOCKS    256  /* 32x4 + 8x16 */
#define M1_TRAILER_MOD  4    /* block % 4 == 3 is the sector trailer (1K) */

/* Authenticate with Key A on the sector containing block_addr.
 * uid: 4 or 7 byte UID from the poll. */
OPERATE_RET m1_auth_a(uint8_t block_addr, const uint8_t key[6],
                      const uint8_t *uid, uint8_t uid_len);

/* Read one 16-byte block. Requires prior auth on that sector. */
OPERATE_RET m1_read_block(uint8_t block_addr, uint8_t out[M1_BLOCK_LEN]);

/* Write one 16-byte block. Requires prior auth on that sector. */
OPERATE_RET m1_write_block(uint8_t block_addr, const uint8_t data[M1_BLOCK_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* MIFARE_CLASSIC_H */
