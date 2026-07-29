#include <string.h>

#include "tal_api.h"

#include "ntag.h"

#define NTAG_CMD_GET_VERSION 0x60U
#define NTAG_CMD_READ        0x30U
#define NTAG_CMD_WRITE       0xA2U

#define NTAG_TIMEOUT_MS 800U

ntag_type_e ntag_get_version(void)
{
    uint8_t tx[1] = {NTAG_CMD_GET_VERSION};
    uint8_t rx[8];
    uint8_t rx_len = sizeof(rx);

    if (pn532_apdu_transceive(tx, sizeof(tx), rx, &rx_len, NTAG_TIMEOUT_MS) != OPRT_OK) {
        return NTAG_TYPE_UNKNOWN;
    }
    /* GET_VERSION answer: header(00 04) vendor(04 NXP) type(04) subtype len */
    if (rx_len < 8U || rx[0] != 0x00U || rx[1] != 0x04U || rx[2] != 0x04U) {
        return NTAG_TYPE_UNKNOWN;
    }
    switch (rx[6]) { /* storage size byte */
    case 0x0F: return NTAG_TYPE_213;
    case 0x11: return NTAG_TYPE_215;
    case 0x13: return NTAG_TYPE_216;
    default:   return NTAG_TYPE_UNKNOWN;
    }
}

uint8_t ntag_user_last_page(ntag_type_e type)
{
    switch (type) {
    case NTAG_TYPE_213: return 39;
    case NTAG_TYPE_215: return 129;
    case NTAG_TYPE_216: return 225;
    default:            return 39;
    }
}

OPERATE_RET ntag_read(uint8_t page, uint8_t out[NTAG_READ_LEN])
{
    uint8_t tx[2] = {NTAG_CMD_READ, page};
    uint8_t rx[NTAG_READ_LEN + 2];
    uint8_t rx_len = sizeof(rx);
    OPERATE_RET rt;

    rt = pn532_apdu_transceive(tx, sizeof(tx), rx, &rx_len, NTAG_TIMEOUT_MS);
    if (rt != OPRT_OK) {
        return rt;
    }
    if (rx_len < NTAG_READ_LEN) {
        return OPRT_COM_ERROR;
    }
    memcpy(out, rx, NTAG_READ_LEN);
    return OPRT_OK;
}

OPERATE_RET ntag_write_page(uint8_t page, const uint8_t data[NTAG_PAGE_LEN])
{
    uint8_t tx[2 + NTAG_PAGE_LEN];
    uint8_t rx[4];
    uint8_t rx_len = sizeof(rx);

    tx[0] = NTAG_CMD_WRITE;
    tx[1] = page;
    memcpy(&tx[2], data, NTAG_PAGE_LEN);
    return pn532_apdu_transceive(tx, sizeof(tx), rx, &rx_len, NTAG_TIMEOUT_MS);
}
