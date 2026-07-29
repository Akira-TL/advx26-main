#include <stdio.h>
#include <string.h>

#include "tal_api.h"
#include "cJSON.h"
#include "lvgl.h"

#include "master_link.h"
#include "tcp_link.h"

static slave_status_t sg_slave;
static bool sg_inited;

/* Raw rx line ring for the COMM screen log. Single producer (dispatcher
 * timer) and single consumer (COMM screen timer), both on the LVGL thread,
 * so no locking is needed. */
static char sg_log_ring[MASTER_LINK_LOG_DEPTH][MASTER_LINK_LOG_LINE_MAX];
static uint8_t sg_log_head;
static uint8_t sg_log_tail;

static void mlink_log_push(const char *line)
{
    uint8_t next = (uint8_t)((sg_log_head + 1U) % MASTER_LINK_LOG_DEPTH);

    if (next == sg_log_tail) {
        sg_log_tail = (uint8_t)((sg_log_tail + 1U) % MASTER_LINK_LOG_DEPTH);
    }
    strncpy(sg_log_ring[sg_log_head], line, MASTER_LINK_LOG_LINE_MAX - 1);
    sg_log_ring[sg_log_head][MASTER_LINK_LOG_LINE_MAX - 1] = '\0';
    sg_log_head = next;
}

BOOL_T master_link_pop_log(char *buf)
{
    if (sg_log_tail == sg_log_head) {
        return FALSE;
    }
    strncpy(buf, sg_log_ring[sg_log_tail], MASTER_LINK_LOG_LINE_MAX);
    buf[MASTER_LINK_LOG_LINE_MAX - 1] = '\0';
    sg_log_tail = (uint8_t)((sg_log_tail + 1U) % MASTER_LINK_LOG_DEPTH);
    return TRUE;
}

static slave_state_e mlink_state_from_name(const char *name)
{
    if (strcmp(name, "ready") == 0) return SLAVE_STATE_READY;
    if (strcmp(name, "playing") == 0) return SLAVE_STATE_PLAYING;
    if (strcmp(name, "paused") == 0) return SLAVE_STATE_PAUSED;
    if (strcmp(name, "buffering") == 0) return SLAVE_STATE_BUFFERING;
    if (strcmp(name, "completed") == 0) return SLAVE_STATE_COMPLETED;
    if (strcmp(name, "error") == 0) return SLAVE_STATE_ERROR;
    if (strcmp(name, "idle") == 0) return SLAVE_STATE_IDLE;
    return SLAVE_STATE_UNKNOWN;
}

static void mlink_handle_line(const char *line)
{
    cJSON *root = cJSON_Parse(line);
    cJSON *type;
    cJSON *item;

    if (root == NULL) {
        return;
    }
    type = cJSON_GetObjectItem(root, "type");
    if (!cJSON_IsString(type)) {
        cJSON_Delete(root);
        return;
    }

    if (strcmp(type->valuestring, "state") == 0) {
        item = cJSON_GetObjectItem(root, "state");
        if (cJSON_IsString(item)) {
            sg_slave.state = mlink_state_from_name(item->valuestring);
        }
        item = cJSON_GetObjectItem(root, "position_ms");
        if (cJSON_IsNumber(item)) {
            sg_slave.position_ms = (uint32_t)item->valuedouble;
        }
        item = cJSON_GetObjectItem(root, "duration_ms");
        if (cJSON_IsNumber(item)) {
            sg_slave.duration_ms = (uint32_t)item->valuedouble;
        }
    } else if (strcmp(type->valuestring, "ack") == 0) {
        if (sg_slave.state == SLAVE_STATE_UNKNOWN ||
            sg_slave.state == SLAVE_STATE_IDLE ||
            sg_slave.state == SLAVE_STATE_COMPLETED ||
            sg_slave.state == SLAVE_STATE_ERROR) {
            sg_slave.state = SLAVE_STATE_BUFFERING;
        }
    } else if (strcmp(type->valuestring, "error") == 0) {
        sg_slave.state = SLAVE_STATE_ERROR;
    }
    /* Unknown types are ignored. */
    cJSON_Delete(root);
}

static void mlink_timer_cb(lv_timer_t *timer)
{
    static char line[TCP_LINK_LINE_MAX];
    uint16_t len;

    (void)timer;

    if (tcp_link_get_state() != TCP_LINK_CONNECTED) {
        if (sg_slave.state != SLAVE_STATE_UNKNOWN) {
            memset(&sg_slave, 0, sizeof(sg_slave));
        }
        return;
    }

    while (tcp_link_poll(line, &len)) {
        mlink_log_push(line);
        mlink_handle_line(line);
    }
}

void master_link_init(void)
{
    if (sg_inited) {
        return;
    }
    sg_inited = true;
    lv_timer_create(mlink_timer_cb, 100, NULL);
}

slave_status_t master_link_slave_status(void)
{
    return sg_slave;
}

void master_link_note_load(void)
{
    /* A new load was just sent: drop stale state so an old ready/paused
     * report cannot trigger a premature start. */
    memset(&sg_slave, 0, sizeof(sg_slave));
    sg_slave.state = SLAVE_STATE_BUFFERING;
}

bool master_link_connected(void)
{
    return tcp_link_get_state() == TCP_LINK_CONNECTED;
}

static bool mlink_send_line(const char *json)
{
    uint16_t len = (uint16_t)strlen(json);

    if (tcp_link_send(json, len) != OPRT_OK ||
        tcp_link_send("\n", 1) != OPRT_OK) {
        return false;
    }
    return true;
}

bool master_link_send_start(void)
{
    return mlink_send_line("{\"type\":\"start\"}");
}

bool master_link_send_pause(void)
{
    return mlink_send_line("{\"type\":\"pause\"}");
}

bool master_link_send_resume(void)
{
    return mlink_send_line("{\"type\":\"resume\"}");
}

bool master_link_send_stop(void)
{
    return mlink_send_line("{\"type\":\"stop\"}");
}

bool master_link_send_raw(const char *text, uint16_t len)
{
    if (tcp_link_send(text, len) != OPRT_OK ||
        tcp_link_send("\n", 1) != OPRT_OK) {
        return false;
    }
    return true;
}
