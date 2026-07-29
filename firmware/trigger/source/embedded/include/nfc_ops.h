#ifndef NFC_OPS_H
#define NFC_OPS_H

#include "nfc_card.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Poll for any supported card and read its identity + memory.
 * Fills card; type is set even when the memory is unreadable.
 * Returns OPRT_NOT_FOUND when nothing is in the field.
 */
OPERATE_RET nfc_ops_read_card(nfc_card_t *card);

/*
 * Full read (type probe + memory dump) of a card already selected by
 * nfc_ops_detect(), without re-polling. Card must hold the detect result.
 */
OPERATE_RET nfc_ops_read_detected(nfc_card_t *card);

/*
 * Lightweight presence check: a single ISO14443-A poll, no retries and no
 * memory dump. Fills only uid/sak/atqa. Meant for the background resident
 * scan so the bit-banged I2C bus stays quiet when no card is present.
 */
OPERATE_RET nfc_ops_detect(nfc_card_t *card);

typedef enum {
    NFC_NDEF_TEXT = 0,
    NFC_NDEF_URI,
    NFC_NDEF_AAR,   /* Android Application Record */
} nfc_ndef_type_e;

/*
 * Poll for a writable card and store a record on it.
 * NTAG/Ultralight: written as an NDEF record of the given type
 * (Text / URI / AAR, phone-readable).
 * Mifare Classic: raw text, zero-padded, into sector 1 blocks 4..6
 * (the typed string itself is stored, regardless of record type).
 * out_type (optional) receives the card type that was written.
 * Returns OPRT_NOT_FOUND / OPRT_NOT_SUPPORTED / OPRT_COM_ERROR (auth fail).
 */
OPERATE_RET nfc_ops_write_record(nfc_ndef_type_e ndef_type, const char *text,
                                 nfc_card_type_e *out_type);

/* Short display name for a card type. */
const char *nfc_card_type_name(nfc_card_type_e type);

#ifdef __cplusplus
}
#endif

#endif /* NFC_OPS_H */
