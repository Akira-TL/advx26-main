#include <string.h>

#include "tal_api.h"

#include "felica.h"

#define FELICA_CMD_READ_WO_ENC  0x06U
#define FELICA_CMD_REQ_SYSCODE  0x0CU

#define FELICA_TIMEOUT_MS 600U

OPERATE_RET felica_request_system_code(const uint8_t *idm, uint16_t *systems,
                                       uint8_t *count)
{
    uint8_t tx[10];
    uint8_t rx[40];
    uint8_t rx_len = sizeof(rx);
    uint8_t max_count;
    uint8_t n;
    uint8_t i;
    OPERATE_RET rt;

    if (idm == NULL || systems == NULL || count == NULL || *count == 0U) {
        return OPRT_INVALID_PARM;
    }
    max_count = *count;

    tx[0] = 10; /* LEN: cmd + IDm */
    tx[1] = FELICA_CMD_REQ_SYSCODE;
    memcpy(&tx[2], idm, PN532_IDM_LEN);

    rt = pn532_felica_transceive(tx, sizeof(tx), rx, &rx_len, FELICA_TIMEOUT_MS);
    if (rt != OPRT_OK) {
        return rt;
    }
    /* rx: [len][0x0D][IDm8][count][sc_lo sc_hi]*count */
    if (rx_len < 11U || rx[1] != (FELICA_CMD_REQ_SYSCODE + 1U)) {
        return OPRT_COM_ERROR;
    }
    n = rx[10];
    if (n > max_count || (uint16_t)(11U + n * 2U) > rx_len) {
        return OPRT_COM_ERROR;
    }
    for (i = 0; i < n; i++) {
        systems[i] = (uint16_t)((uint16_t)rx[11U + i * 2U] |
                                ((uint16_t)rx[12U + i * 2U] << 8U));
    }
    *count = n;
    return OPRT_OK;
}

OPERATE_RET felica_read_block(const uint8_t *idm, uint16_t service,
                              uint8_t addr, uint8_t out[FELICA_BLOCK_LEN])
{
    uint8_t tx[16];
    uint8_t rx[40];
    uint8_t rx_len = sizeof(rx);
    OPERATE_RET rt;

    if (idm == NULL || out == NULL) {
        return OPRT_INVALID_PARM;
    }

    tx[0] = 15; /* LEN: everything after this byte */
    tx[1] = FELICA_CMD_READ_WO_ENC;
    memcpy(&tx[2], idm, PN532_IDM_LEN);
    tx[10] = 0x01; /* one service */
    tx[11] = (uint8_t)(service & 0xFFU);
    tx[12] = (uint8_t)(service >> 8U);
    tx[13] = 0x01; /* one block */
    tx[14] = 0x80; /* 2-byte block list element */
    tx[15] = addr;

    rt = pn532_felica_transceive(tx, sizeof(tx), rx, &rx_len, FELICA_TIMEOUT_MS);
    if (rt != OPRT_OK) {
        return rt;
    }
    /* rx: [len][0x07][IDm8][SF1][SF2][N][16*N data] */
    if (rx_len < 13U || rx[1] != (FELICA_CMD_READ_WO_ENC + 1U)) {
        return OPRT_COM_ERROR;
    }
    if (rx[10] != 0x00U || rx[11] != 0x00U || rx[12] == 0x00U) {
        return OPRT_NOT_FOUND; /* status flag: read past end / error */
    }
    memcpy(out, &rx[13], FELICA_BLOCK_LEN);
    return OPRT_OK;
}
