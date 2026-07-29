#include "tal_api.h"
#include "tkl_uart.h"
#include "board_link.h"

#define LINK_UART_PORT      TUYA_UART_NUM_0
#define LINK_UART_BAUD      115200

typedef struct {
    char buf[BOARD_LINK_LINE_MAX];
    uint16_t len;
} ring_slot_t;

static ring_slot_t s_ring[BOARD_LINK_RING_DEPTH];
static volatile uint8_t s_ring_head;
static volatile uint8_t s_ring_tail;

static char s_rx_buf[BOARD_LINK_LINE_MAX];
static uint16_t s_rx_pos;

static void __link_rx_cb(TUYA_UART_NUM_E port)
{
    uint8_t byte;

    while (tkl_uart_read(port, &byte, 1) > 0) {
        if (byte == '\n') {
            if (s_rx_pos > 0) {
                uint8_t next = (uint8_t)((s_ring_head + 1U) % BOARD_LINK_RING_DEPTH);
                if (next != s_ring_tail) {
                    s_rx_buf[s_rx_pos] = '\0';
                    memcpy(s_ring[s_ring_head].buf, s_rx_buf, s_rx_pos + 1U);
                    s_ring[s_ring_head].len = s_rx_pos;
                    s_ring_head = next;
                }
            }
            s_rx_pos = 0;
        } else if (byte != '\r') {
            if (s_rx_pos < BOARD_LINK_LINE_MAX - 1) {
                s_rx_buf[s_rx_pos++] = (char)byte;
            } else {
                s_rx_pos = 0;
            }
        }
    }
}

OPERATE_RET board_link_init(void)
{
    s_rx_pos = 0;
    s_ring_head = 0;
    s_ring_tail = 0;

    TUYA_UART_BASE_CFG_T cfg = {
        .baudrate = LINK_UART_BAUD,
        .parity   = TUYA_UART_PARITY_TYPE_NONE,
        .databits = TUYA_UART_DATA_LEN_8BIT,
        .stopbits = TUYA_UART_STOP_LEN_1BIT,
        .flowctrl = TUYA_UART_FLOWCTRL_NONE,
    };

    OPERATE_RET rt = tkl_uart_init(LINK_UART_PORT, &cfg);
    if (rt != OPRT_OK)
        return rt;

    tkl_uart_rx_irq_cb_reg(LINK_UART_PORT, __link_rx_cb);
    return OPRT_OK;
}

OPERATE_RET board_link_send(const char *data, uint16_t len)
{
    int32_t written = tkl_uart_write(LINK_UART_PORT, (void *)data, len);
    return (written > 0) ? OPRT_OK : OPRT_COM_ERROR;
}

BOOL_T board_link_poll(char *buf, uint16_t *len)
{
    if (s_ring_tail == s_ring_head)
        return FALSE;

    *len = s_ring[s_ring_tail].len;
    memcpy(buf, s_ring[s_ring_tail].buf, *len + 1U);
    s_ring_tail = (uint8_t)((s_ring_tail + 1U) % BOARD_LINK_RING_DEPTH);
    return TRUE;
}
