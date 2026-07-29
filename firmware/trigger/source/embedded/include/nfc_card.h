#ifndef NFC_CARD_H
#define NFC_CARD_H

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NFC_UID_MAX_LEN   10
#define NFC_DUMP_MAX_LEN  1024
#define NFC_BLOCK_MAX     128
#define NFC_NDEF_TEXT_MAX 128

typedef enum {
    NFC_CARD_NONE = 0,
    NFC_CARD_MIFARE_1K,
    NFC_CARD_MIFARE_4K,
    NFC_CARD_NTAG,
    NFC_CARD_ULTRALIGHT,
    NFC_CARD_ISO_DEP,
    NFC_CARD_FELICA,
    NFC_CARD_TYPE_B,
    NFC_CARD_UNKNOWN,
} nfc_card_type_e;

typedef struct {
    nfc_card_type_e type;
    uint8_t uid[NFC_UID_MAX_LEN];
    uint8_t uid_len;
    uint8_t atqa[2];
    uint8_t sak;
    uint16_t block_count;   /* M1: 16B blocks; NTAG: 4B pages (dump uses 16B rows) */
    uint16_t size_bytes;
    uint8_t dump[NFC_DUMP_MAX_LEN];
    uint8_t readable[NFC_BLOCK_MAX]; /* per 16B row: 1 = data valid */
    uint16_t dump_rows;              /* number of 16B rows in dump */
    char ndef_text[NFC_NDEF_TEXT_MAX];
    char content_id[33];             /* 32-hex SoundPola id from /c/<id>, or empty */
} nfc_card_t;

#ifdef __cplusplus
}
#endif

#endif /* NFC_CARD_H */
