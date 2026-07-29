#ifndef TCP_LINK_H
#define TCP_LINK_H

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Max length of one received/transmitted line (excluding NUL). */
#define TCP_LINK_LINE_MAX   2200
/* Number of received lines buffered for the UI to drain. */
#define TCP_LINK_RING_DEPTH 8
/* Port this link listens on for incoming peer connections. Must differ from
 * the pairing HTTP server port (8787, ui_pair.c). */
#define TCP_LINK_PORT       8788

typedef enum {
    TCP_LINK_IDLE = 0,   /* not connected, no attempt in flight */
    TCP_LINK_CONNECTING, /* connect() in progress */
    TCP_LINK_CONNECTED,  /* socket established */
    TCP_LINK_FAILED,     /* last connect attempt failed / link dropped */
} tcp_link_state_e;

/*
 * Start the link worker thread. Call once at startup. The worker also listens
 * on TCP_LINK_PORT and accepts one incoming peer connection when idle, so two
 * devices running this firmware can talk with a single Connect on either side.
 */
OPERATE_RET tcp_link_init(void);

/*
 * Request an (asynchronous) connect to ip:port. Ignored unless the link is
 * idle/failed. The worker performs the blocking connect off the UI thread.
 */
void tcp_link_connect(const char *ip, uint16_t port);

/* Request a disconnect. Safe to call from the UI thread. */
void tcp_link_disconnect(void);

/* Current link state (safe to read from the UI thread). */
tcp_link_state_e tcp_link_get_state(void);

/*
 * Send raw bytes over the established socket. Returns OPRT_COM_ERROR when not
 * connected. Caller appends any framing (e.g. '\n').
 */
OPERATE_RET tcp_link_send(const char *data, uint16_t len);

/*
 * Pop one received '\n'-terminated line (NUL-stripped) into buf. Returns TRUE
 * and sets *len when a line was available, FALSE when the queue is empty.
 */
BOOL_T tcp_link_poll(char *buf, uint16_t *len);

/*
 * Cumulative bytes received on the socket since boot. Lets the UI confirm that
 * data is arriving even when it has not (yet) been framed into a line.
 */
uint32_t tcp_link_rx_bytes(void);

/* Cumulative bytes handed to the socket for transmission since boot. */
uint32_t tcp_link_tx_bytes(void);

#ifdef __cplusplus
}
#endif

#endif /* TCP_LINK_H */
