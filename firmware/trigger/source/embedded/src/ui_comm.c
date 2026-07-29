#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tal_api.h"
#include "tal_wifi.h"
#include "lvgl.h"

#include "ui_comm.h"
#include "ui_nav.h"
#include "tcp_link.h"
#include "master_link.h"

#define COMM_BG         0x0A1628
#define COMM_CARD       0x1B2838
#define COMM_RX_COLOR   0x4ADE80
#define COMM_TX_COLOR   0x60A5FA
#define COMM_WHITE      0xFFFFFF
#define COMM_GRAY       0x888888
#define COMM_MUTED      0xA9C1D7
#define COMM_GREEN      0x4ADE80
#define COMM_YELLOW     0xFFD166
#define COMM_RED        0xFF6B7A

#define LOG_AREA_X      8
#define LOG_AREA_W      464
#define OWNIP_TOP       42
#define PEER_TOP        64
#define PEER_H          32
#define PRESET_TOP      100
#define PRESET_H        32
/* Log sits below the peer row when the keyboard is up (presets hidden), and
 * below the preset row when it is down. */
#define LOG_TOP_OPEN    (PEER_TOP + PEER_H + 4)
#define LOG_TOP_CLOSED  (PRESET_TOP + PRESET_H + 4)
#define INPUT_H         40
#define KB_H            150

#define DEFAULT_PORT    TCP_LINK_PORT

static lv_obj_t *sg_scr;
static lv_obj_t *sg_log_list;
static lv_obj_t *sg_textarea;
static lv_obj_t *sg_keyboard;
static lv_obj_t *sg_kb_btn;
static lv_obj_t *sg_kb_btn_label;
static lv_obj_t *sg_input_bar;
static lv_obj_t *sg_preset_row;
static lv_obj_t *sg_ownip_label;
static lv_obj_t *sg_conn_status;
static lv_obj_t *sg_peer_ip_ta;
static lv_obj_t *sg_port_ta;
static lv_obj_t *sg_connect_btn;
static lv_obj_t *sg_connect_btn_label;

static bool sg_kb_visible;
static int sg_last_link_state = -1;
static char sg_last_ip[20];
static uint32_t sg_last_rx_bytes;
static uint32_t sg_last_tx_bytes;

/* Preset NDJSON messages for the board-to-board link. Tapping a chip copies
 * the payload into the input so it can be edited before sending. */
static const struct {
    const char *label;
    const char *msg;
} sg_presets[] = {
    {"status", "{\"type\":\"status\"}"},
    {"state",  "{\"type\":\"state\"}"},
    {"ack",    "{\"type\":\"ack\",\"state\":\"accepted\"}"},
    {"load",   "{\"type\":\"load\",\"content_id\":\"\"}"},
};

static void comm_append_msg(const char *prefix, const char *text, uint32_t color)
{
    char buf[TCP_LINK_LINE_MAX + 4];
    lv_obj_t *label;

    snprintf(buf, sizeof(buf), "%s %s", prefix, text);
    PR_NOTICE("comm log: %s", buf);

    label = lv_label_create(sg_log_list);
    lv_label_set_text(label, buf);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_width(label, LOG_AREA_W - 32);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    lv_obj_scroll_to_y(sg_log_list, LV_COORD_MAX, LV_ANIM_ON);
}

static void comm_update_conn_ui(void)
{
    tcp_link_state_e st = tcp_link_get_state();

    if ((int)st == sg_last_link_state)
        return;
    sg_last_link_state = (int)st;

    switch (st) {
    case TCP_LINK_CONNECTED:
        lv_label_set_text(sg_conn_status, "Connected");
        lv_obj_set_style_text_color(sg_conn_status, lv_color_hex(COMM_GREEN), LV_PART_MAIN);
        lv_label_set_text(sg_connect_btn_label, "Disconnect");
        lv_obj_clear_state(sg_connect_btn, LV_STATE_DISABLED);
        break;
    case TCP_LINK_CONNECTING:
        lv_label_set_text(sg_conn_status, "Connecting...");
        lv_obj_set_style_text_color(sg_conn_status, lv_color_hex(COMM_YELLOW), LV_PART_MAIN);
        lv_label_set_text(sg_connect_btn_label, "Connecting");
        lv_obj_add_state(sg_connect_btn, LV_STATE_DISABLED);
        break;
    case TCP_LINK_FAILED:
        lv_label_set_text(sg_conn_status, "Connect failed");
        lv_obj_set_style_text_color(sg_conn_status, lv_color_hex(COMM_RED), LV_PART_MAIN);
        lv_label_set_text(sg_connect_btn_label, "Connect");
        lv_obj_clear_state(sg_connect_btn, LV_STATE_DISABLED);
        break;
    default:
        lv_label_set_text(sg_conn_status, "Disconnected");
        lv_obj_set_style_text_color(sg_conn_status, lv_color_hex(COMM_MUTED), LV_PART_MAIN);
        lv_label_set_text(sg_connect_btn_label, "Connect");
        lv_obj_clear_state(sg_connect_btn, LV_STATE_DISABLED);
        break;
    }
}

static void comm_update_own_ip(void)
{
    WF_STATION_STAT_E stat = WSS_IDLE;
    NW_IP_S ip;
    char buf[64];
    const char *ip_str = "--";
    uint32_t rx = tcp_link_rx_bytes();
    uint32_t tx = tcp_link_tx_bytes();

    if (tal_wifi_station_get_status(&stat) == OPRT_OK && stat == WSS_GOT_IP) {
        memset(&ip, 0, sizeof(ip));
        if (tal_wifi_get_ip(WF_STATION, &ip) == OPRT_OK && ip.ip[0] != '\0')
            ip_str = ip.ip;
    }

    if (strcmp(ip_str, sg_last_ip) == 0 && rx == sg_last_rx_bytes &&
        tx == sg_last_tx_bytes)
        return;
    strncpy(sg_last_ip, ip_str, sizeof(sg_last_ip) - 1);
    sg_last_ip[sizeof(sg_last_ip) - 1] = '\0';
    sg_last_rx_bytes = rx;
    sg_last_tx_bytes = tx;

    snprintf(buf, sizeof(buf), "My IP: %s  RX:%u TX:%u", ip_str,
             (unsigned)rx, (unsigned)tx);
    lv_label_set_text(sg_ownip_label, buf);
}

static void comm_poll_timer_cb(lv_timer_t *timer)
{
    static char poll_buf[MASTER_LINK_LOG_LINE_MAX];

    (void)timer;
    if (lv_screen_active() != sg_scr)
        return;

    while (master_link_pop_log(poll_buf)) {
        comm_append_msg(">", poll_buf, COMM_RX_COLOR);
    }

    comm_update_own_ip();
    comm_update_conn_ui();
}

static void comm_preset_cb(lv_event_t *e)
{
    int idx = (int)(uintptr_t)lv_event_get_user_data(e);

    if (LV_EVENT_CLICKED != lv_event_get_code(e))
        return;
    lv_textarea_set_text(sg_textarea, sg_presets[idx].msg);
}

static void comm_send_cb(lv_event_t *e)
{
    const char *text;
    uint16_t len;

    if (LV_EVENT_CLICKED != lv_event_get_code(e))
        return;

    text = lv_textarea_get_text(sg_textarea);
    len = (uint16_t)strlen(text);
    if (len == 0) {
        comm_append_msg("!", "empty input", COMM_GRAY);
        return;
    }

    if (tcp_link_send(text, len) != OPRT_OK ||
        tcp_link_send("\n", 1) != OPRT_OK) {
        comm_append_msg("!", "not connected", COMM_GRAY);
        return;
    }
    comm_append_msg("<", text, COMM_TX_COLOR);
    lv_textarea_set_text(sg_textarea, "");
}

static void comm_connect_cb(lv_event_t *e)
{
    tcp_link_state_e st;
    const char *ip;
    long port;

    if (LV_EVENT_CLICKED != lv_event_get_code(e))
        return;

    st = tcp_link_get_state();
    if (st == TCP_LINK_CONNECTED) {
        tcp_link_disconnect();
        return;
    }
    if (st == TCP_LINK_CONNECTING)
        return;

    ip = lv_textarea_get_text(sg_peer_ip_ta);
    port = strtol(lv_textarea_get_text(sg_port_ta), NULL, 10);
    if (port <= 0 || port > 65535)
        port = DEFAULT_PORT;
    tcp_link_connect(ip, (uint16_t)port);
}

static void comm_update_layout(void)
{
    int32_t input_top, log_top;

    if (sg_kb_visible) {
        input_top = 320 - KB_H - INPUT_H;
        lv_obj_set_pos(sg_input_bar, 0, input_top);
        lv_obj_set_pos(sg_keyboard, 0, 320 - KB_H);
        lv_obj_clear_flag(sg_keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(sg_preset_row, LV_OBJ_FLAG_HIDDEN);
        log_top = LOG_TOP_OPEN;
    } else {
        input_top = 320 - INPUT_H;
        lv_obj_set_pos(sg_input_bar, 0, input_top);
        lv_obj_set_pos(sg_keyboard, 0, 320);
        lv_obj_add_flag(sg_keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sg_preset_row, LV_OBJ_FLAG_HIDDEN);
        log_top = LOG_TOP_CLOSED;
    }
    lv_obj_set_pos(sg_log_list, LOG_AREA_X, log_top);
    lv_obj_set_size(sg_log_list, LOG_AREA_W, (input_top - 8) - log_top);
}

static void comm_kb_toggle_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e))
        return;

    sg_kb_visible = !sg_kb_visible;
    comm_update_layout();
}

/* Focus handler shared by all textareas: point the keyboard at the focused
 * field and reveal it. */
static void comm_ta_focus_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta;

    if (code != LV_EVENT_CLICKED && code != LV_EVENT_FOCUSED)
        return;

    ta = lv_event_get_target(e);
    lv_keyboard_set_textarea(sg_keyboard, ta);
    /* IP/port fields get the numeric pad; the message field gets the full
     * text keyboard. */
    lv_keyboard_set_mode(sg_keyboard,
                         (ta == sg_peer_ip_ta || ta == sg_port_ta)
                             ? LV_KEYBOARD_MODE_NUMBER
                             : LV_KEYBOARD_MODE_TEXT_LOWER);
    if (!sg_kb_visible) {
        sg_kb_visible = true;
        comm_update_layout();
    }
}

static void comm_back_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e))
        return;
    ui_nav_go_anim(UI_SCR_HOME, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 250);
}

static lv_obj_t *comm_make_ta(lv_obj_t *parent, int32_t x, int32_t w,
                              const char *placeholder, uint32_t max_len)
{
    lv_obj_t *ta = lv_textarea_create(parent);

    lv_obj_set_pos(ta, x, 0);
    lv_obj_set_size(ta, w, PEER_H);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_placeholder_text(ta, placeholder);
    lv_textarea_set_max_length(ta, max_len);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0x0F1D2E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ta, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(ta, lv_color_hex(COMM_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(ta, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_border_width(ta, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(ta, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_left(ta, 8, LV_PART_MAIN);
    lv_obj_add_event_cb(ta, comm_ta_focus_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ta, comm_ta_focus_cb, LV_EVENT_FOCUSED, NULL);
    return ta;
}

void ui_comm_init(void)
{
    lv_obj_t *btn, *lbl;
    char port_str[8];

    tcp_link_init();
    master_link_init();

    sg_scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(sg_scr);
    lv_obj_set_style_bg_color(sg_scr, lv_color_hex(COMM_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(sg_scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(sg_scr, LV_OBJ_FLAG_CLICKABLE);

    /* Top bar: back button */
    btn = lv_button_create(sg_scr);
    lv_obj_set_pos(btn, 4, 4);
    lv_obj_set_size(btn, 32, 32);
    lv_obj_set_style_radius(btn, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COMM_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, comm_back_cb, LV_EVENT_CLICKED, NULL);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(lbl, lv_color_hex(COMM_WHITE), LV_PART_MAIN);
    lv_obj_center(lbl);

    /* Title */
    lbl = lv_label_create(sg_scr);
    lv_label_set_text(lbl, "COMM");
    lv_obj_set_pos(lbl, 44, 8);
    lv_obj_set_style_text_color(lbl, lv_color_hex(COMM_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_add_flag(lbl, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(lbl, LV_OBJ_FLAG_USER_1);

    /* Keyboard toggle button */
    sg_kb_btn = lv_button_create(sg_scr);
    lv_obj_set_pos(sg_kb_btn, 440, 4);
    lv_obj_set_size(sg_kb_btn, 36, 32);
    lv_obj_set_style_radius(sg_kb_btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_kb_btn, lv_color_hex(COMM_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_kb_btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(sg_kb_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(sg_kb_btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(sg_kb_btn, comm_kb_toggle_cb, LV_EVENT_CLICKED, NULL);
    sg_kb_btn_label = lv_label_create(sg_kb_btn);
    lv_label_set_text(sg_kb_btn_label, LV_SYMBOL_KEYBOARD);
    lv_obj_set_style_text_color(sg_kb_btn_label, lv_color_hex(COMM_WHITE), LV_PART_MAIN);
    lv_obj_center(sg_kb_btn_label);

    /* Own IP + connection status row */
    sg_ownip_label = lv_label_create(sg_scr);
    lv_label_set_text(sg_ownip_label, "My IP: --");
    lv_obj_set_pos(sg_ownip_label, 8, OWNIP_TOP);
    lv_obj_set_style_text_color(sg_ownip_label, lv_color_hex(COMM_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(sg_ownip_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_add_flag(sg_ownip_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(sg_ownip_label, LV_OBJ_FLAG_USER_1);

    sg_conn_status = lv_label_create(sg_scr);
    lv_label_set_text(sg_conn_status, "Disconnected");
    lv_obj_set_pos(sg_conn_status, 352, OWNIP_TOP);
    lv_obj_set_width(sg_conn_status, 120);
    lv_obj_set_style_text_align(sg_conn_status, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_text_color(sg_conn_status, lv_color_hex(COMM_MUTED), LV_PART_MAIN);
    lv_obj_set_style_text_font(sg_conn_status, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_add_flag(sg_conn_status, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(sg_conn_status, LV_OBJ_FLAG_USER_1);

    /* Peer row: peer IP, port, connect button */
    sg_peer_ip_ta = comm_make_ta(sg_scr, 8, 250, "Peer IP (e.g. 192.168.1.10)", 15);
    sg_port_ta = comm_make_ta(sg_scr, 262, 56, "Port", 5);
    snprintf(port_str, sizeof(port_str), "%d", DEFAULT_PORT);
    lv_textarea_set_text(sg_port_ta, port_str);
    lv_obj_set_pos(sg_peer_ip_ta, 8, PEER_TOP);
    lv_obj_set_pos(sg_port_ta, 262, PEER_TOP);

    sg_connect_btn = lv_button_create(sg_scr);
    lv_obj_set_pos(sg_connect_btn, 322, PEER_TOP);
    lv_obj_set_size(sg_connect_btn, 150, PEER_H);
    lv_obj_set_style_radius(sg_connect_btn, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_connect_btn, lv_color_hex(0x2563EB), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_connect_btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(sg_connect_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(sg_connect_btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(sg_connect_btn, comm_connect_cb, LV_EVENT_CLICKED, NULL);
    sg_connect_btn_label = lv_label_create(sg_connect_btn);
    lv_label_set_text(sg_connect_btn_label, "Connect");
    lv_obj_set_style_text_color(sg_connect_btn_label, lv_color_hex(COMM_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(sg_connect_btn_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_center(sg_connect_btn_label);

    /* Preset message chips (tap to fill the input) */
    sg_preset_row = lv_obj_create(sg_scr);
    lv_obj_remove_style_all(sg_preset_row);
    lv_obj_set_pos(sg_preset_row, 0, PRESET_TOP);
    lv_obj_set_size(sg_preset_row, 480, PRESET_H);
    lv_obj_clear_flag(sg_preset_row, LV_OBJ_FLAG_SCROLLABLE);
    {
        int n = (int)(sizeof(sg_presets) / sizeof(sg_presets[0]));
        int w = (480 - 8 * (n + 1)) / n;
        int i;

        for (i = 0; i < n; i++) {
            btn = lv_button_create(sg_preset_row);
            lv_obj_set_pos(btn, 8 + i * (w + 8), 0);
            lv_obj_set_size(btn, w, PRESET_H);
            lv_obj_set_style_radius(btn, 6, LV_PART_MAIN);
            lv_obj_set_style_bg_color(btn, lv_color_hex(COMM_CARD), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
            lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
            lv_obj_add_event_cb(btn, comm_preset_cb, LV_EVENT_CLICKED,
                                (void *)(uintptr_t)i);
            lbl = lv_label_create(btn);
            lv_label_set_text(lbl, sg_presets[i].label);
            lv_obj_set_style_text_color(lbl, lv_color_hex(COMM_TX_COLOR), LV_PART_MAIN);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_center(lbl);
        }
    }

    /* Message log area: plain scrollable flex column (same pattern as the
     * NFC dump list, which renders reliably); labels are added directly. */
    sg_log_list = lv_obj_create(sg_scr);
    lv_obj_set_style_bg_color(sg_log_list, lv_color_hex(COMM_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_log_list, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(sg_log_list, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(sg_log_list, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(sg_log_list, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(sg_log_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(sg_log_list, LV_SCROLLBAR_MODE_AUTO);

    /* Input bar container */
    sg_input_bar = lv_obj_create(sg_scr);
    lv_obj_remove_style_all(sg_input_bar);
    lv_obj_set_size(sg_input_bar, 480, INPUT_H);
    lv_obj_set_style_bg_color(sg_input_bar, lv_color_hex(COMM_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_input_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(sg_input_bar, LV_OBJ_FLAG_SCROLLABLE);

    /* Message textarea */
    sg_textarea = lv_textarea_create(sg_input_bar);
    lv_obj_set_pos(sg_textarea, 4, 4);
    lv_obj_set_size(sg_textarea, 396, 32);
    lv_textarea_set_one_line(sg_textarea, true);
    lv_textarea_set_placeholder_text(sg_textarea, "Enter message...");
    lv_textarea_set_max_length(sg_textarea, TCP_LINK_LINE_MAX - 1);
    lv_obj_set_style_bg_color(sg_textarea, lv_color_hex(0x0F1D2E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_textarea, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(sg_textarea, lv_color_hex(COMM_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(sg_textarea, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_border_width(sg_textarea, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(sg_textarea, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_left(sg_textarea, 8, LV_PART_MAIN);
    lv_obj_add_event_cb(sg_textarea, comm_ta_focus_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(sg_textarea, comm_ta_focus_cb, LV_EVENT_FOCUSED, NULL);

    /* Send button */
    btn = lv_button_create(sg_input_bar);
    lv_obj_set_pos(btn, 406, 4);
    lv_obj_set_size(btn, 68, 32);
    lv_obj_set_style_radius(btn, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2563EB), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, comm_send_cb, LV_EVENT_CLICKED, NULL);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Send");
    lv_obj_set_style_text_color(lbl, lv_color_hex(COMM_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_center(lbl);

    /* Keyboard (hidden by default). The keyboard class defaults to
     * LV_ALIGN_BOTTOM_MID — reset to TOP_LEFT so set_pos is absolute. */
    sg_keyboard = lv_keyboard_create(sg_scr);
    lv_obj_set_align(sg_keyboard, LV_ALIGN_TOP_LEFT);
    lv_obj_set_size(sg_keyboard, 480, KB_H);
    lv_keyboard_set_textarea(sg_keyboard, sg_textarea);
    lv_obj_set_style_bg_color(sg_keyboard, lv_color_hex(COMM_CARD), LV_PART_MAIN);
    lv_obj_set_style_text_font(sg_keyboard, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_keyboard, lv_color_hex(0x0F1D2E), LV_PART_ITEMS);
    lv_obj_set_style_text_color(sg_keyboard, lv_color_hex(COMM_WHITE), LV_PART_ITEMS);
    lv_obj_set_style_text_font(sg_keyboard, &lv_font_montserrat_14, LV_PART_ITEMS);
    lv_obj_add_flag(sg_keyboard, LV_OBJ_FLAG_HIDDEN);

    sg_kb_visible = false;
    sg_last_link_state = -1;
    sg_last_ip[0] = '\0';
    comm_update_layout();

    lv_timer_create(comm_poll_timer_cb, 100, NULL);

    ui_nav_register(UI_SCR_COMM, sg_scr);
    ui_nav_attach_gesture(sg_scr);
}
