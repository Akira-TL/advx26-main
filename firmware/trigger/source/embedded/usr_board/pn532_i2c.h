#ifndef PN532_I2C_H
#define PN532_I2C_H

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PN532_UID_MAX_LEN 10
#define PN532_IDM_LEN     8

typedef enum {
    PN532_SCAN_ERROR = -1,
    PN532_SCAN_NOT_FOUND = 0,
    PN532_SCAN_FOUND = 1,
} PN532_SCAN_RESULT_E;

typedef enum {
    PN532_POLL_ISO14443A = 0x00,
    PN532_POLL_FELICA_212 = 0x01,
    PN532_POLL_ISO14443B = 0x03,
} PN532_POLL_TYPE_E;

typedef struct {
    uint8_t uid[PN532_UID_MAX_LEN]; /* A: UID, FeliCa: IDm, B: PUPI */
    uint8_t uid_len;
    uint8_t sak;                    /* ISO14443-A only */
    uint8_t atqa[2];                /* ISO14443-A only */
} PN532_TARGET_T;

OPERATE_RET pn532_i2c_init(void);
PN532_SCAN_RESULT_E pn532_scan_passive_target(uint8_t *uid, uint8_t *uid_len);
const char *pn532_i2c_diagnostic(void);

/*
 * Poll one passive target of the given technology. For FeliCa the IDm is
 * returned in target->uid (8 bytes); for ISO14443-B the PUPI (4 bytes).
 * Returns OPRT_NOT_FOUND when no card is in the field.
 */
OPERATE_RET pn532_poll_target(PN532_POLL_TYPE_E type, PN532_TARGET_T *target);

/*
 * InCommunicateThru: raw frame exchange with the target last polled
 * (FeliCa path; PN532 handles the RF-level CRC). rx capacity is passed in
 * *rx_len and the received length is written back.
 */
OPERATE_RET pn532_felica_transceive(const uint8_t *tx, uint8_t tx_len,
                                    uint8_t *rx, uint8_t *rx_len,
                                    uint32_t timeout_ms);

/*
 * InDataExchange: ISO14443-4 APDU exchange with the Type-A/B target last
 * polled. The trailing SW1/SW2 are kept in rx.
 */
OPERATE_RET pn532_apdu_transceive(const uint8_t *tx, uint8_t tx_len,
                                  uint8_t *rx, uint8_t *rx_len,
                                  uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* PN532_I2C_H */
