#include <string.h>

#include "tal_api.h"

#include "mifare_classic.h"

#define M1_CMD_AUTH_A 0x60U
#define M1_CMD_READ   0x30U
#define M1_CMD_WRITE  0xA0U

#define M1_TIMEOUT_MS 800U

OPERATE_RET m1_auth_a(uint8_t block_addr, const uint8_t key[6],
                      const uint8_t *uid, uint8_t uid_len)
{
    uint8_t tx[12];
    uint8_t rx[8];
    uint8_t rx_len = sizeof(rx);

    if (uid_len < 4U) {
        return OPRT_INVALID_PARM;
    }
    tx[0] = M1_CMD_AUTH_A;
    tx[1] = block_addr;
    memcpy(&tx[2], key, 6);
    /* For 7-byte UIDs the card authenticates with the first 4 bytes */
    memcpy(&tx[8], uid, 4);
    return pn532_apdu_transceive(tx, 12, rx, &rx_len, M1_TIMEOUT_MS);
}

OPERATE_RET m1_read_block(uint8_t block_addr, uint8_t out[M1_BLOCK_LEN])
{
    uint8_t tx[2] = {M1_CMD_READ, block_addr};
    uint8_t rx[M1_BLOCK_LEN + 2];
    uint8_t rx_len = sizeof(rx);
    OPERATE_RET rt;

    rt = pn532_apdu_transceive(tx, sizeof(tx), rx, &rx_len, M1_TIMEOUT_MS);
    if (rt != OPRT_OK) {
        return rt;
    }
    if (rx_len < M1_BLOCK_LEN) {
        return OPRT_COM_ERROR;
    }
    memcpy(out, rx, M1_BLOCK_LEN);
    return OPRT_OK;
}

OPERATE_RET m1_write_block(uint8_t block_addr, const uint8_t data[M1_BLOCK_LEN])
{
    uint8_t tx[2 + M1_BLOCK_LEN];
    uint8_t rx[4];
    uint8_t rx_len = sizeof(rx);

    tx[0] = M1_CMD_WRITE;
    tx[1] = block_addr;
    memcpy(&tx[2], data, M1_BLOCK_LEN);
    return pn532_apdu_transceive(tx, sizeof(tx), rx, &rx_len, M1_TIMEOUT_MS);
}
