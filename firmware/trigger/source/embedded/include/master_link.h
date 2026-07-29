#ifndef MASTER_LINK_H
#define MASTER_LINK_H

#include <stdbool.h>
#include <stdint.h>

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Max chars of one rx line kept in the UI log ring (truncated beyond). */
#define MASTER_LINK_LOG_LINE_MAX 256
#define MASTER_LINK_LOG_DEPTH    8

typedef enum {
    SLAVE_STATE_UNKNOWN = 0, /* nothing heard yet / link down */
    SLAVE_STATE_IDLE,
    SLAVE_STATE_BUFFERING,   /* load acked, prefetching */
    SLAVE_STATE_READY,       /* prefetch done, can start instantly */
    SLAVE_STATE_PLAYING,
    SLAVE_STATE_PAUSED,
    SLAVE_STATE_COMPLETED,
    SLAVE_STATE_ERROR,
} slave_state_e;

typedef struct {
    slave_state_e state;
    uint32_t position_ms;
    uint32_t duration_ms;
} slave_status_t;

/*
 * Start the dispatcher. Creates an lv_timer that drains tcp_link_poll()
 * regardless of which screen is active, parses NDJSON from the slave and
 * updates the slave status. Call once from the UI thread after tcp_link_init.
 */
void master_link_init(void);

/* Latest known slave status (UI thread only). */
slave_status_t master_link_slave_status(void);

/* Reset cached slave status right after sending a new load, so stale
 * ready/paused reports cannot trigger a premature start. */
void master_link_note_load(void);

/*
 * Pop one raw received line into buf (size >= MASTER_LINK_LOG_LINE_MAX).
 * Returns TRUE when a line was available. Used by ui_comm for its log.
 */
BOOL_T master_link_pop_log(char *buf);

/* Send helpers. All return FALSE when the link is not connected. */
bool master_link_send_start(void);
bool master_link_send_pause(void);
bool master_link_send_resume(void);
bool master_link_send_stop(void);

/* Send a raw line (ui_comm manual input). Appends '\n'. */
bool master_link_send_raw(const char *text, uint16_t len);

/* TRUE when the tcp link is connected to the slave. */
bool master_link_connected(void);

#ifdef __cplusplus
}
#endif

#endif /* MASTER_LINK_H */
