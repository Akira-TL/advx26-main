#include <string.h>

#include "tal_api.h"
#include "tkl_gpio.h"
#include "tkl_system.h"

#include "pn532_i2c.h"

#define PN532_I2C_SCL_PIN        TUYA_GPIO_NUM_2
#define PN532_I2C_SDA_PIN        TUYA_GPIO_NUM_3
#define PN532_I2C_ADDRESS        0x24U
#define PN532_I2C_DELAY_US       10U
#define PN532_I2C_READY          0x01U
#define PN532_HOST_TO_PN532      0xD4U
#define PN532_PN532_TO_HOST      0xD5U
#define PN532_CMD_GET_FIRMWARE   0x02U
#define PN532_CMD_SAM_CONFIG     0x14U
#define PN532_CMD_RF_CONFIG      0x32U
#define PN532_CMD_DATA_EXCHANGE  0x40U
#define PN532_CMD_COMM_THRU      0x42U
#define PN532_CMD_LIST_TARGET    0x4AU
#define PN532_FRAME_MAX_LEN      64U
#define PN532_RESPONSE_MAX_LEN   160U
#define PN532_READY_POLL_MS      10U
#define PN532_PREPARE_RETRIES    3U
#define PN532_STRETCH_TIMEOUT_US 5000U
#define PN532_ADDR_RETRIES       3U

static const uint8_t sg_ack_frame[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
static bool sg_pn532_ready;
static uint8_t sg_last_status;
static const char *sg_diagnostic = "I2C starting";

static void i2c_delay(void)
{
    tkl_system_sleep_us(PN532_I2C_DELAY_US);
}

static void i2c_scl_low(void)
{
    TUYA_GPIO_BASE_CFG_T cfg = {
        .mode = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_OUTPUT,
        .level = TUYA_GPIO_LEVEL_LOW,
    };

    tkl_gpio_init(PN532_I2C_SCL_PIN, &cfg);
    i2c_delay();
}

/*
 * PN532 stretches SCL while byte processing. Release SCL as a pulled-up
 * input and wait until the line really rises before continuing.
 */
static void i2c_scl_release(void)
{
    TUYA_GPIO_BASE_CFG_T cfg = {
        .mode = TUYA_GPIO_PULLUP,
        .direct = TUYA_GPIO_INPUT,
        .level = TUYA_GPIO_LEVEL_HIGH,
    };
    TUYA_GPIO_LEVEL_E level = TUYA_GPIO_LEVEL_LOW;
    uint32_t waited_us = 0;

    tkl_gpio_init(PN532_I2C_SCL_PIN, &cfg);
    while (waited_us < PN532_STRETCH_TIMEOUT_US) {
        tkl_gpio_read(PN532_I2C_SCL_PIN, &level);
        if (level == TUYA_GPIO_LEVEL_HIGH) {
            break;
        }
        tkl_system_sleep_us(PN532_I2C_DELAY_US);
        waited_us += PN532_I2C_DELAY_US;
    }
    i2c_delay();
}

/*
 * T5AI's GPIO adapter does not implement open-drain output. Implement it by
 * driving SDA low for zero and switching SDA to pulled-up input for one.
 */
static void i2c_sda_low(void)
{
    TUYA_GPIO_BASE_CFG_T cfg = {
        .mode = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_OUTPUT,
        .level = TUYA_GPIO_LEVEL_LOW,
    };

    tkl_gpio_init(PN532_I2C_SDA_PIN, &cfg);
    i2c_delay();
}

static void i2c_sda_release(void)
{
    TUYA_GPIO_BASE_CFG_T cfg = {
        .mode = TUYA_GPIO_PULLUP,
        .direct = TUYA_GPIO_INPUT,
        .level = TUYA_GPIO_LEVEL_HIGH,
    };

    tkl_gpio_init(PN532_I2C_SDA_PIN, &cfg);
    i2c_delay();
}

static TUYA_GPIO_LEVEL_E i2c_sda_read(void)
{
    TUYA_GPIO_LEVEL_E level = TUYA_GPIO_LEVEL_HIGH;

    tkl_gpio_read(PN532_I2C_SDA_PIN, &level);
    return level;
}

static void i2c_start(void)
{
    i2c_sda_release();
    i2c_scl_release();
    i2c_sda_low();
    i2c_scl_low();
}

static void i2c_stop(void)
{
    i2c_sda_low();
    i2c_scl_release();
    i2c_sda_release();
}

/*
 * An MCU reset can interrupt a read mid-byte, leaving the PN532 holding
 * SDA low forever. Clock SCL with SDA released until the slave lets go,
 * then finish with a stop condition.
 */
static void i2c_bus_recover(void)
{
    uint8_t i;

    i2c_sda_release();
    for (i = 0; i < 9U; i++) {
        if (i2c_sda_read() == TUYA_GPIO_LEVEL_HIGH) {
            break;
        }
        i2c_scl_release();
        i2c_scl_low();
    }
    i2c_stop();
}

static bool i2c_write_byte(uint8_t data)
{
    uint8_t mask;
    bool ack;

    for (mask = 0x80U; mask != 0U; mask >>= 1U) {
        if (data & mask) {
            i2c_sda_release();
        } else {
            i2c_sda_low();
        }
        i2c_scl_release();
        i2c_scl_low();
    }

    i2c_sda_release();
    i2c_scl_release();
    ack = (i2c_sda_read() == TUYA_GPIO_LEVEL_LOW);
    i2c_scl_low();
    return ack;
}

static uint8_t i2c_read_byte(bool ack)
{
    uint8_t data = 0;
    uint8_t i;

    i2c_sda_release();
    for (i = 0; i < 8U; i++) {
        data <<= 1U;
        i2c_scl_release();
        if (i2c_sda_read() == TUYA_GPIO_LEVEL_HIGH) {
            data |= 1U;
        }
        i2c_scl_low();
    }

    if (ack) {
        i2c_sda_low();
    } else {
        i2c_sda_release();
    }
    i2c_scl_release();
    i2c_scl_low();
    i2c_sda_release();
    return data;
}

static OPERATE_RET i2c_write(const uint8_t *data, uint8_t len)
{
    uint8_t attempt;
    uint8_t i;
    bool addressed = false;

    for (attempt = 0; attempt < PN532_ADDR_RETRIES; attempt++) {
        i2c_start();
        if (i2c_write_byte((uint8_t)(PN532_I2C_ADDRESS << 1U))) {
            addressed = true;
            break;
        }
        i2c_stop();
        tal_system_sleep(1);
    }
    if (!addressed) {
        return OPRT_COM_ERROR;
    }
    for (i = 0; i < len; i++) {
        if (!i2c_write_byte(data[i])) {
            i2c_stop();
            return OPRT_COM_ERROR;
        }
    }
    i2c_stop();
    return OPRT_OK;
}

static OPERATE_RET i2c_read(uint8_t *data, uint8_t len)
{
    uint8_t attempt;
    uint8_t i;
    bool addressed = false;

    for (attempt = 0; attempt < PN532_ADDR_RETRIES; attempt++) {
        i2c_start();
        if (i2c_write_byte((uint8_t)((PN532_I2C_ADDRESS << 1U) | 1U))) {
            addressed = true;
            break;
        }
        i2c_stop();
        tal_system_sleep(1);
    }
    if (!addressed) {
        return OPRT_COM_ERROR;
    }
    for (i = 0; i < len; i++) {
        data[i] = i2c_read_byte(i + 1U < len);
    }
    i2c_stop();
    return OPRT_OK;
}

static uint8_t pn532_build_frame(uint8_t command, const uint8_t *params,
                                 uint8_t params_len, uint8_t *frame)
{
    uint8_t data_len = (uint8_t)(params_len + 2U);
    uint8_t checksum = (uint8_t)(PN532_HOST_TO_PN532 + command);
    uint8_t index = 0;
    uint8_t i;

    frame[index++] = 0x00;
    frame[index++] = 0x00;
    frame[index++] = 0xFF;
    frame[index++] = data_len;
    frame[index++] = (uint8_t)(0U - data_len);
    frame[index++] = PN532_HOST_TO_PN532;
    frame[index++] = command;
    for (i = 0; i < params_len; i++) {
        frame[index++] = params[i];
        checksum = (uint8_t)(checksum + params[i]);
    }
    frame[index++] = (uint8_t)(0U - checksum);
    frame[index++] = 0x00;
    return index;
}

static OPERATE_RET pn532_read_status(uint8_t *status)
{
    OPERATE_RET rt = i2c_read(status, 1);

    if (rt == OPRT_OK) {
        sg_last_status = *status;
    }
    return rt;
}

static OPERATE_RET pn532_wait_ready(uint32_t timeout_ms)
{
    SYS_TIME_T start = tal_system_get_millisecond();
    uint8_t status;
    OPERATE_RET rt;

    while ((uint32_t)(tal_system_get_millisecond() - start) < timeout_ms) {
        rt = pn532_read_status(&status);
        if (rt == OPRT_OK && status == PN532_I2C_READY) {
            return OPRT_OK;
        }
        tal_system_sleep(PN532_READY_POLL_MS);
    }
    return OPRT_TIMEOUT;
}

static OPERATE_RET pn532_read_frame(uint8_t *data, uint8_t data_len)
{
    uint8_t packet[PN532_RESPONSE_MAX_LEN + 1U];
    OPERATE_RET rt;

    if (data_len > PN532_RESPONSE_MAX_LEN) {
        return OPRT_INVALID_PARM;
    }
    rt = i2c_read(packet, (uint8_t)(data_len + 1U));
    if (rt != OPRT_OK) {
        return rt;
    }
    sg_last_status = packet[0];
    if (packet[0] != PN532_I2C_READY) {
        return OPRT_COM_ERROR;
    }
    memcpy(data, &packet[1], data_len);
    return OPRT_OK;
}

static OPERATE_RET pn532_parse_response(const uint8_t *frame,
                                        uint8_t frame_size,
                                        uint8_t expected_response,
                                        uint8_t *response,
                                        uint8_t *response_len)
{
    uint8_t data_len;
    uint8_t checksum = 0;
    uint8_t copy_len;
    uint8_t i;

    if (frame_size < 9U || frame[0] != 0x00 || frame[1] != 0x00 ||
        frame[2] != 0xFF) {
        return OPRT_COM_ERROR;
    }
    data_len = frame[3];
    if ((uint8_t)(data_len + frame[4]) != 0x00 ||
        data_len < 2U || (uint16_t)data_len + 7U > frame_size ||
        frame[5] != PN532_PN532_TO_HOST ||
        frame[6] != expected_response) {
        return OPRT_COM_ERROR;
    }
    for (i = 0; i <= data_len; i++) {
        checksum = (uint8_t)(checksum + frame[5U + i]);
    }
    if (checksum != 0x00 || frame[(uint8_t)(data_len + 6U)] != 0x00) {
        return OPRT_COM_ERROR;
    }

    copy_len = (uint8_t)(data_len - 1U);
    if (copy_len > *response_len) {
        copy_len = *response_len;
    }
    if (copy_len > 0U && response != NULL) {
        memcpy(response, &frame[6], copy_len);
    }
    *response_len = copy_len;
    return OPRT_OK;
}

static OPERATE_RET pn532_send_command(uint8_t command,
                                      const uint8_t *params,
                                      uint8_t params_len,
                                      uint8_t *response,
                                      uint8_t *response_len,
                                      uint32_t timeout_ms)
{
    uint8_t frame[PN532_FRAME_MAX_LEN];
    uint8_t rx_frame[PN532_RESPONSE_MAX_LEN];
    uint8_t ack[sizeof(sg_ack_frame)] = {0};
    uint8_t frame_len;
    OPERATE_RET rt;

    if (params_len > PN532_FRAME_MAX_LEN - 9U) {
        return OPRT_INVALID_PARM;
    }
    frame_len = pn532_build_frame(command, params, params_len, frame);
    rt = i2c_write(frame, frame_len);
    if (rt != OPRT_OK) {
        sg_diagnostic = "I2C ADDR NAK - check mode/power";
        PR_ERR("PN532 I2C2 write failed: %d", rt);
        return rt;
    }

    rt = pn532_wait_ready(300);
    if (rt != OPRT_OK) {
        sg_diagnostic = "I2C NOT READY - check mode";
        PR_ERR("PN532 I2C ACK timeout, status=0x%02X", sg_last_status);
        return rt;
    }
    rt = pn532_read_frame(ack, sizeof(ack));
    if (rt != OPRT_OK || memcmp(ack, sg_ack_frame, sizeof(ack)) != 0) {
        sg_diagnostic = "I2C ACK INVALID";
        PR_ERR("PN532 I2C ACK: %02X %02X %02X %02X %02X %02X",
               ack[0], ack[1], ack[2], ack[3], ack[4], ack[5]);
        return OPRT_COM_ERROR;
    }

    rt = pn532_wait_ready(timeout_ms);
    if (rt != OPRT_OK) {
        sg_diagnostic = "PN532 RESPONSE TIMEOUT";
        return rt;
    }
    memset(rx_frame, 0, sizeof(rx_frame));
    rt = pn532_read_frame(rx_frame, sizeof(rx_frame));
    if (rt != OPRT_OK) {
        sg_diagnostic = "I2C READ FAILED";
        return rt;
    }
    return pn532_parse_response(rx_frame, sizeof(rx_frame),
                                (uint8_t)(command + 1U),
                                response, response_len);
}

static OPERATE_RET pn532_prepare(void)
{
    static const uint8_t sam_params[] = {0x01, 0x14, 0x01};
    /* MaxRetries: MxRtyATR=0xFF, MxRtyPSL=0x01, MxRtyPassiveActivation=0x01.
     * The power-on default retries passive activation forever, so a no-card
     * InListPassiveTarget only ends via the host timeout (500-1200 ms of
     * busy bit-banged I2C). One retry makes it return "0 targets" in ~50 ms. */
    static const uint8_t rf_retry_params[] = {0x05, 0xFF, 0x01, 0x01};
    uint8_t response[16];
    uint8_t response_len;
    OPERATE_RET rt = OPRT_COM_ERROR;
    uint8_t attempt;

    for (attempt = 0; attempt < PN532_PREPARE_RETRIES; attempt++) {
        response_len = sizeof(response);
        rt = pn532_send_command(PN532_CMD_SAM_CONFIG, sam_params,
                                sizeof(sam_params), response,
                                &response_len, 1000);
        if (rt != OPRT_OK) {
            tal_system_sleep(100);
            continue;
        }
        response_len = sizeof(response);
        rt = pn532_send_command(PN532_CMD_RF_CONFIG, rf_retry_params,
                                sizeof(rf_retry_params), response,
                                &response_len, 1000);
        if (rt != OPRT_OK) {
            tal_system_sleep(100);
            continue;
        }
        response_len = sizeof(response);
        rt = pn532_send_command(PN532_CMD_GET_FIRMWARE, NULL, 0,
                                response, &response_len, 1000);
        if (rt == OPRT_OK && response_len >= 5U) {
            sg_diagnostic = "PN532 I2C ready";
            PR_NOTICE("PN532 I2C online, IC=0x%02X firmware=%u.%u support=0x%02X",
                      response[1], response[2], response[3], response[4]);
            return OPRT_OK;
        }
        tal_system_sleep(100);
    }
    return (rt == OPRT_OK) ? OPRT_COM_ERROR : rt;
}

OPERATE_RET pn532_i2c_init(void)
{
    TUYA_GPIO_LEVEL_E scl_idle = TUYA_GPIO_LEVEL_LOW;
    TUYA_GPIO_LEVEL_E sda_idle = TUYA_GPIO_LEVEL_LOW;
    uint8_t status = 0;
    uint8_t attempt;
    OPERATE_RET rt = OPRT_COM_ERROR;

    i2c_scl_release();
    i2c_bus_recover();

    sg_pn532_ready = false;
    sg_last_status = 0;
    sg_diagnostic = "Waiting for PN532 I2C";

    tkl_gpio_read(PN532_I2C_SCL_PIN, &scl_idle);
    tkl_gpio_read(PN532_I2C_SDA_PIN, &sda_idle);
    PR_NOTICE("PN532 I2C idle: SCL(P02)=%d SDA(P03)=%d",
              (int)scl_idle, (int)sda_idle);

    for (attempt = 0; attempt < 5U; attempt++) {
        tal_system_sleep(500);
        rt = pn532_read_status(&status);
        if (rt == OPRT_OK) {
            PR_NOTICE("PN532 address 0x24 ACK, status=0x%02X (attempt %u)",
                      status, attempt + 1U);
            break;
        }
        PR_NOTICE("PN532 address 0x24 NAK, attempt %u/5", attempt + 1U);
    }
    if (rt != OPRT_OK) {
        sg_diagnostic = "I2C ADDR NAK - check mode/power";
        PR_ERR("PN532 address 0x24 NAK after 5 attempts");
    }
    PR_NOTICE("PN532 timed software I2C: SCL=P02 SDA=P03 address=0x24");
    return OPRT_OK;
}

const char *pn532_i2c_diagnostic(void)
{
    return sg_diagnostic;
}

PN532_SCAN_RESULT_E pn532_scan_passive_target(uint8_t *uid, uint8_t *uid_len)
{
    static const uint8_t list_target[] = {0x01, 0x00};
    uint8_t response[32];
    uint8_t response_len = sizeof(response);
    uint8_t found_uid_len;
    OPERATE_RET rt;

    if (uid == NULL || uid_len == NULL) {
        return PN532_SCAN_ERROR;
    }
    *uid_len = 0;
    if (!sg_pn532_ready) {
        rt = pn532_prepare();
        if (rt != OPRT_OK) {
            return PN532_SCAN_ERROR;
        }
        sg_pn532_ready = true;
    }

    rt = pn532_send_command(PN532_CMD_LIST_TARGET, list_target,
                            sizeof(list_target), response, &response_len, 1200);
    if (rt == OPRT_TIMEOUT) {
        return PN532_SCAN_NOT_FOUND;
    }
    if (rt != OPRT_OK || response_len < 2U) {
        sg_pn532_ready = false;
        sg_diagnostic = "PN532 RESPONSE INVALID";
        return PN532_SCAN_ERROR;
    }
    if (response[1] == 0U) {
        return PN532_SCAN_NOT_FOUND;
    }
    if (response_len < 7U) {
        sg_pn532_ready = false;
        return PN532_SCAN_ERROR;
    }

    found_uid_len = response[6];
    if (found_uid_len == 0U || found_uid_len > PN532_UID_MAX_LEN ||
        (uint16_t)found_uid_len + 7U > response_len) {
        sg_pn532_ready = false;
        return PN532_SCAN_ERROR;
    }
    memcpy(uid, &response[7], found_uid_len);
    *uid_len = found_uid_len;
    return PN532_SCAN_FOUND;
}

OPERATE_RET pn532_poll_target(PN532_POLL_TYPE_E type, PN532_TARGET_T *target)
{
    /* SENSF_REQ: cmd 0x00, wildcard system code 0xFFFF, request PMm, slot 0 */
    static const uint8_t felica_params[] = {0x01, PN532_POLL_FELICA_212,
                                            0x00, 0xFF, 0xFF, 0x01, 0x00};
    /* MaxTg=1, BrTy=B, AFI=0 */
    static const uint8_t isob_params[] = {0x01, PN532_POLL_ISO14443B, 0x00};
    uint8_t typea_params[] = {0x01, PN532_POLL_ISO14443A};
    const uint8_t *params;
    uint8_t params_len;
    uint8_t response[48];
    uint8_t response_len = sizeof(response);
    OPERATE_RET rt;

    if (target == NULL) {
        return OPRT_INVALID_PARM;
    }
    memset(target, 0, sizeof(*target));

    if (PN532_POLL_FELICA_212 == type) {
        params = felica_params;
        params_len = sizeof(felica_params);
    } else if (PN532_POLL_ISO14443B == type) {
        params = isob_params;
        params_len = sizeof(isob_params);
    } else {
        typea_params[1] = (uint8_t)type;
        params = typea_params;
        params_len = sizeof(typea_params);
    }

    if (!sg_pn532_ready) {
        rt = pn532_prepare();
        if (rt != OPRT_OK) {
            return rt;
        }
        sg_pn532_ready = true;
    }

    rt = pn532_send_command(PN532_CMD_LIST_TARGET, params, params_len,
                            response, &response_len, 500);
    if (rt == OPRT_TIMEOUT) {
        return OPRT_NOT_FOUND;
    }
    /* With MaxRetries configured, "no card" is a real 2-byte response
     * [0x4B][NbTg=0], not a timeout. */
    if (rt != OPRT_OK || response_len < 2U) {
        sg_pn532_ready = false;
        sg_diagnostic = "PN532 POLL FAILED";
        return (rt == OPRT_OK) ? OPRT_COM_ERROR : rt;
    }
    if (response[1] == 0U) {
        return OPRT_NOT_FOUND;
    }

    if (PN532_POLL_ISO14443A == (uint8_t)type) {
        /* resp: [0x4B][NbTg][Tg][ATQA2][SAK][UIDlen][UID...] */
        if (response_len < 7U || response[6] == 0U ||
            response[6] > PN532_UID_MAX_LEN ||
            (uint16_t)response[6] + 7U > response_len) {
            return OPRT_COM_ERROR;
        }
        target->atqa[0] = response[3];
        target->atqa[1] = response[4];
        target->sak = response[5];
        target->uid_len = response[6];
        memcpy(target->uid, &response[7], target->uid_len);
    } else if (PN532_POLL_FELICA_212 == (uint8_t)type) {
        /* resp: [0x4B][NbTg][Tg][len][0x01][IDm8][PMm8] */
        if (response_len < 13U || response[4] != 0x01U) {
            return OPRT_COM_ERROR;
        }
        memcpy(target->uid, &response[5], PN532_IDM_LEN);
        target->uid_len = PN532_IDM_LEN;
    } else {
        /* ISO14443-B resp: [0x4B][NbTg][Tg][ATQB...] with PUPI at ATQB[1..4] */
        if (response_len < 8U) {
            return OPRT_COM_ERROR;
        }
        memcpy(target->uid, &response[4], 4);
        target->uid_len = 4;
    }
    return OPRT_OK;
}

OPERATE_RET pn532_felica_transceive(const uint8_t *tx, uint8_t tx_len,
                                    uint8_t *rx, uint8_t *rx_len,
                                    uint32_t timeout_ms)
{
    uint8_t response[PN532_RESPONSE_MAX_LEN];
    uint8_t response_len = sizeof(response);
    OPERATE_RET rt;

    if (tx == NULL || rx == NULL || rx_len == NULL || tx_len == 0U) {
        return OPRT_INVALID_PARM;
    }
    rt = pn532_send_command(PN532_CMD_COMM_THRU, tx, tx_len,
                            response, &response_len, timeout_ms);
    if (rt != OPRT_OK) {
        return rt;
    }
    /* resp: [0x43][status][data...] */
    if (response_len < 2U || response[1] != 0x00U) {
        sg_diagnostic = "FELICA THRU ERROR";
        return OPRT_COM_ERROR;
    }
    if ((uint8_t)(response_len - 2U) > *rx_len) {
        return OPRT_COM_ERROR;
    }
    memcpy(rx, &response[2], (uint8_t)(response_len - 2U));
    *rx_len = (uint8_t)(response_len - 2U);
    return OPRT_OK;
}

OPERATE_RET pn532_apdu_transceive(const uint8_t *tx, uint8_t tx_len,
                                  uint8_t *rx, uint8_t *rx_len,
                                  uint32_t timeout_ms)
{
    uint8_t params[PN532_FRAME_MAX_LEN - 9U];
    uint8_t response[PN532_RESPONSE_MAX_LEN];
    uint8_t response_len = sizeof(response);
    OPERATE_RET rt;

    if (tx == NULL || rx == NULL || rx_len == NULL || tx_len == 0U ||
        tx_len > sizeof(params) - 1U) {
        return OPRT_INVALID_PARM;
    }
    params[0] = 0x01; /* Tg */
    memcpy(&params[1], tx, tx_len);

    rt = pn532_send_command(PN532_CMD_DATA_EXCHANGE, params,
                            (uint8_t)(tx_len + 1U), response,
                            &response_len, timeout_ms);
    if (rt != OPRT_OK) {
        return rt;
    }
    /* resp: [0x41][status][data...]; status 0x00 = success */
    if (response_len < 2U || response[1] != 0x00U) {
        sg_diagnostic = "APDU EXCHANGE ERROR";
        return OPRT_COM_ERROR;
    }
    if ((uint8_t)(response_len - 2U) > *rx_len) {
        return OPRT_COM_ERROR;
    }
    memcpy(rx, &response[2], (uint8_t)(response_len - 2U));
    *rx_len = (uint8_t)(response_len - 2U);
    return OPRT_OK;
}
