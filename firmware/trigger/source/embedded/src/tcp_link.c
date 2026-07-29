#include <string.h>

#include "tal_api.h"
#include "tal_network.h"

#include "tcp_link.h"

#define LINK_RX_CHUNK      512
#define LINK_SELECT_MS     200   /* recv poll granularity; bounds disconnect latency */
#define LINK_IDLE_SLEEP_MS 50

typedef struct {
    char buf[TCP_LINK_LINE_MAX];
    uint16_t len;
} ring_slot_t;

static ring_slot_t s_ring[TCP_LINK_RING_DEPTH];
static volatile uint8_t s_ring_head;
static volatile uint8_t s_ring_tail;

static char s_rx_line[TCP_LINK_LINE_MAX];
static uint16_t s_rx_pos;

static THREAD_HANDLE s_thread;
static int s_fd = -1;
static int s_listen_fd = -1;
static volatile tcp_link_state_e s_state = TCP_LINK_IDLE;
static volatile BOOL_T s_connect_req;
static volatile BOOL_T s_disconnect_req;
static char s_peer_ip[32];
static uint16_t s_peer_port;
static volatile uint32_t s_rx_bytes;
static volatile uint32_t s_tx_bytes;

static void ring_push(const char *line, uint16_t len)
{
    uint8_t next = (uint8_t)((s_ring_head + 1U) % TCP_LINK_RING_DEPTH);

    if (next == s_ring_tail) {
        return; /* full: drop the newest line */
    }
    memcpy(s_ring[s_ring_head].buf, line, len + 1U);
    s_ring[s_ring_head].len = len;
    s_ring_head = next;
}

/* Split a chunk of received bytes into '\n'-terminated lines. */
static void link_feed(const char *data, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        char c = data[i];

        if (c == '\n') {
            if (s_rx_pos > 0U) {
                s_rx_line[s_rx_pos] = '\0';
                PR_NOTICE("tcp_link: rx line (%u B): %s",
                          (unsigned)s_rx_pos, s_rx_line);
                ring_push(s_rx_line, s_rx_pos);
            }
            s_rx_pos = 0;
        } else if (c != '\r') {
            if (s_rx_pos < TCP_LINK_LINE_MAX - 1U) {
                s_rx_line[s_rx_pos++] = c;
            } else {
                s_rx_pos = 0; /* over-long line: restart */
            }
        }
    }
}

static void link_close(void)
{
    if (s_fd >= 0) {
        tal_net_close(s_fd);
        s_fd = -1;
    }
}

/* Pump the established socket (s_fd) until disconnect request, peer close or
 * error. Returns TRUE when the loop ended because of a disconnect request. */
static BOOL_T link_rx_loop(void)
{
    char rbuf[LINK_RX_CHUNK];

    while (!s_disconnect_req) {
        TUYA_FD_SET_T rfds;
        int r;

        TAL_FD_ZERO(&rfds);
        TAL_FD_SET(s_fd, &rfds);
        r = tal_net_select(s_fd + 1, &rfds, NULL, NULL, LINK_SELECT_MS);
        if (r <= 0) {
            continue; /* timeout: loop and re-check the disconnect flag */
        }
        if (!TAL_FD_ISSET(s_fd, &rfds)) {
            continue;
        }

        r = tal_net_recv(s_fd, rbuf, sizeof(rbuf));
        if (r > 0) {
            s_rx_bytes += (uint32_t)r;
            link_feed(rbuf, r);
        } else {
            /* 0 = peer closed, <0 = error: tear the link down either way. */
            break;
        }
    }
    return s_disconnect_req;
}

static void link_run_connection(void)
{
    TUYA_IP_ADDR_T addr;

    s_state = TCP_LINK_CONNECTING;
    s_fd = tal_net_socket_create(PROTOCOL_TCP);
    if (s_fd < 0) {
        s_state = TCP_LINK_FAILED;
        return;
    }

    addr = tal_net_str2addr(s_peer_ip);
    if (tal_net_connect(s_fd, addr, s_peer_port) != 0) {
        link_close();
        s_state = TCP_LINK_FAILED;
        return;
    }

    s_state = TCP_LINK_CONNECTED;
    PR_NOTICE("tcp_link: connected to %s:%u", s_peer_ip, s_peer_port);

    {
        BOOL_T requested = link_rx_loop();

        link_close();
        s_state = requested ? TCP_LINK_IDLE : TCP_LINK_FAILED;
        PR_NOTICE("tcp_link: disconnected (state=%d)", s_state);
    }
}

static void link_ensure_listener(void)
{
    if (s_listen_fd >= 0) {
        return;
    }
    s_listen_fd = tal_net_socket_create(PROTOCOL_TCP);
    if (s_listen_fd < 0) {
        return;
    }
    tal_net_set_reuse(s_listen_fd);
    if (tal_net_bind(s_listen_fd, TY_IPADDR_ANY, TCP_LINK_PORT) != 0 ||
        tal_net_listen(s_listen_fd, 1) != 0) {
        tal_net_close(s_listen_fd);
        s_listen_fd = -1;
        return;
    }
    PR_NOTICE("tcp_link: listening on port %u", (unsigned)TCP_LINK_PORT);
}

/* Accept one inbound peer connection and pump it until it drops. */
static void link_run_accepted(void)
{
    TUYA_IP_ADDR_T peer_addr;
    uint16_t peer_port = 0;
    int cfd;

    cfd = tal_net_accept(s_listen_fd, &peer_addr, &peer_port);
    if (cfd < 0) {
        return;
    }

    s_fd = cfd;
    s_disconnect_req = FALSE;
    s_state = TCP_LINK_CONNECTED;
    PR_NOTICE("tcp_link: accepted peer connection (port %u)",
              (unsigned)peer_port);

    link_rx_loop();
    link_close();
    s_state = TCP_LINK_IDLE;
    PR_NOTICE("tcp_link: peer connection closed");
}

static void link_worker(void *arg)
{
    (void)arg;

    while (1) {
        if (s_connect_req) {
            s_connect_req = FALSE;
            s_disconnect_req = FALSE;
            link_run_connection();
            continue;
        }

        link_ensure_listener();
        if (s_listen_fd >= 0) {
            TUYA_FD_SET_T rfds;
            int r;

            TAL_FD_ZERO(&rfds);
            TAL_FD_SET(s_listen_fd, &rfds);
            r = tal_net_select(s_listen_fd + 1, &rfds, NULL, NULL,
                               LINK_SELECT_MS);
            if (r > 0 && TAL_FD_ISSET(s_listen_fd, &rfds)) {
                link_run_accepted();
            }
        } else {
            tal_system_sleep(LINK_IDLE_SLEEP_MS);
        }
    }
}

OPERATE_RET tcp_link_init(void)
{
    THREAD_CFG_T cfg = {
        .stackDepth = 1024 * 6,
        .priority = THREAD_PRIO_2,
        .thrdname = "tcp_link",
    };

    s_ring_head = 0;
    s_ring_tail = 0;
    s_rx_pos = 0;

    return tal_thread_create_and_start(&s_thread, NULL, NULL, link_worker,
                                       NULL, &cfg);
}

void tcp_link_connect(const char *ip, uint16_t port)
{
    if (ip == NULL || ip[0] == '\0') {
        return;
    }
    if (s_state == TCP_LINK_CONNECTING || s_state == TCP_LINK_CONNECTED) {
        return;
    }

    strncpy(s_peer_ip, ip, sizeof(s_peer_ip) - 1U);
    s_peer_ip[sizeof(s_peer_ip) - 1U] = '\0';
    s_peer_port = port;
    s_disconnect_req = FALSE;
    s_connect_req = TRUE;
}

void tcp_link_disconnect(void)
{
    s_connect_req = FALSE;
    s_disconnect_req = TRUE;
}

tcp_link_state_e tcp_link_get_state(void)
{
    return s_state;
}

OPERATE_RET tcp_link_send(const char *data, uint16_t len)
{
    int fd = s_fd;
    int n;

    if (fd < 0 || s_state != TCP_LINK_CONNECTED) {
        return OPRT_COM_ERROR;
    }
    n = tal_net_send(fd, data, len);
    if (n > 0) {
        s_tx_bytes += (uint32_t)n;
        return OPRT_OK;
    }
    return OPRT_COM_ERROR;
}

BOOL_T tcp_link_poll(char *buf, uint16_t *len)
{
    if (s_ring_tail == s_ring_head) {
        return FALSE;
    }
    *len = s_ring[s_ring_tail].len;
    memcpy(buf, s_ring[s_ring_tail].buf, *len + 1U);
    s_ring_tail = (uint8_t)((s_ring_tail + 1U) % TCP_LINK_RING_DEPTH);
    return TRUE;
}

uint32_t tcp_link_rx_bytes(void)
{
    return s_rx_bytes;
}

uint32_t tcp_link_tx_bytes(void)
{
    return s_tx_bytes;
}
