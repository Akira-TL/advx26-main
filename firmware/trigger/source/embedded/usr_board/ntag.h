#ifndef NTAG_H
#define NTAG_H

#include "pn532_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NTAG_PAGE_LEN       4
#define NTAG_USER_START     4    /* first user-memory page on NTAG21x */
#define NTAG_READ_LEN       16   /* one READ returns 4 pages */
#define NTAG_MAX_PAGES      231  /* NTAG216 total */
#define NTAG_UL_LAST_PAGE   15   /* original Ultralight: user pages 4..15 */

typedef enum {
    NTAG_TYPE_UNKNOWN = 0,
    NTAG_TYPE_213,   /* 45 pages total, user 4..39  */
    NTAG_TYPE_215,   /* 135 pages, user 4..129 */
    NTAG_TYPE_216,   /* 231 pages, user 4..225 */
} ntag_type_e;

/* GET_VERSION (0x60). Returns the NTAG model, NTAG_TYPE_UNKNOWN if not a
 * NTAG21x (e.g. plain Ultralight answers with an error). */
ntag_type_e ntag_get_version(void);

/* Last user page (inclusive) for the given model. */
uint8_t ntag_user_last_page(ntag_type_e type);

/* Read 4 pages starting at page into out[16]. */
OPERATE_RET ntag_read(uint8_t page, uint8_t out[NTAG_READ_LEN]);

/* Write one 4-byte page. */
OPERATE_RET ntag_write_page(uint8_t page, const uint8_t data[NTAG_PAGE_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* NTAG_H */
