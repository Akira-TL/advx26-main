#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "tal_api.h"
#include "lvgl.h"

#include "app_nfc.h"
#include "pn532_i2c.h"
#include "nfc_ops.h"
#include "ui_nav.h"
#include "ui_servo.h"
#include "ui_home.h"
#include "ui_playlist.h"
#include "playlist_store.h"

/* Landscape 480x320 (BOARD_LCD_ROTATION = 90) */

#define NFC_BG_TOP    0x071426
#define NFC_BG_BOTTOM 0x153C60
#define NFC_CARD      0x173852
#define NFC_WHITE     0xF7FBFF
#define NFC_MUTED     0xA9C1D7
#define NFC_CYAN      0x48D8FF
#define NFC_GREEN     0x48E0A4
#define NFC_YELLOW    0xFFD166
#define NFC_RED       0xFF6B7A

#define WRITE_TEXT_MAX 100
#define NFC_AUTO_SCAN_MS 1000U

static lv_obj_t *sg_screen_read;
static lv_obj_t *sg_screen_write;

static lv_obj_t *sg_status_dot;
static lv_obj_t *sg_status_label;
static lv_obj_t *sg_type_label;
static lv_obj_t *sg_uid_label;
static lv_obj_t *sg_attr_label;
static lv_obj_t *sg_ndef_caption;
static lv_obj_t *sg_ndef_label;
static lv_obj_t *sg_dump_list;
static lv_obj_t *sg_scan_btn;

static lv_obj_t *sg_w_status_label;
static lv_obj_t *sg_textarea;
static lv_obj_t *sg_keyboard;
static lv_obj_t *sg_write_btn;
static lv_obj_t *sg_type_btns[3];
static uint8_t sg_ndef_type; /* nfc_ndef_type_e */

static THREAD_HANDLE sg_worker;

static volatile bool sg_mode_write;
static volatile bool sg_scan_requested;
static volatile bool sg_write_requested;
static volatile bool sg_result_pending;
static volatile bool sg_result_is_write;
static volatile bool sg_auto_scan_enabled;
static volatile bool sg_scan_full;
static volatile uint8_t sg_miss_count;
static volatile OPERATE_RET sg_op_rt;
static volatile nfc_card_type_e sg_write_type;
static nfc_card_t sg_card;
static char sg_write_text[WRITE_TEXT_MAX + 1];
static volatile uint8_t sg_write_ndef_type;
static bool sg_card_present;
static nfc_card_type_e sg_last_card_type;
static uint8_t sg_last_uid_len;
static uint8_t sg_last_uid[NFC_UID_MAX_LEN];
static char sg_pending_cid[PLIST_ID_MAX];
static uint32_t sg_pending_gen;

/* Blank-card write flow (song picker -> write URL -> eject). */
#define NFC_WCARD_MAX_ATTEMPTS 3U
static volatile bool sg_wcard_active;
static uint8_t sg_wcard_attempts;
static char sg_wcard_url[WRITE_TEXT_MAX + 1];
static lv_timer_t *sg_wcard_retry_timer;

static void nfc_remove_default_style(lv_obj_t *obj)
{
    lv_obj_remove_style_all(obj);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
}

static lv_obj_t *nfc_create_label(lv_obj_t *parent, const char *text,
                                  const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    /* Decorative labels must not block background swipes; we want the press
     * to bubble to the screen-level gesture handler. Marking them CLICKABLE
     * here is harmless because the gesture filter checks USER_1 first and
     * USER_1 wins over CLICKABLE. The PRESSED-state style is the default so
     * swiping across the label does not flash a "pressed" color. */
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(label, LV_OBJ_FLAG_USER_1);
    return label;
}

static lv_obj_t *nfc_create_panel(lv_obj_t *parent, int32_t x, int32_t y,
                                  int32_t w, int32_t h)
{
    lv_obj_t *panel = lv_obj_create(parent);

    nfc_remove_default_style(panel);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, w, h);
    lv_obj_set_style_radius(panel, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(panel, lv_color_hex(NFC_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x5D7891), LV_PART_MAIN);
    lv_obj_set_style_border_opa(panel, LV_OPA_40, LV_PART_MAIN);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    /* Panel = decorative container. Mark USER_1 so the gesture filter lets
     * swipes that started on the panel pass through to the screen gesture. */
    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_USER_1);
    return panel;
}

static void nfc_set_status(uint32_t color, const char *text)
{
    if (sg_mode_write) {
        lv_label_set_text(sg_w_status_label, text);
    } else {
        lv_obj_set_style_bg_color(sg_status_dot, lv_color_hex(color),
                                  LV_PART_MAIN);
        lv_label_set_text(sg_status_label, text);
    }
}

static void nfc_uid_to_text(const nfc_card_t *card, char *out, size_t size)
{
    size_t off = 0;
    uint8_t i;

    for (i = 0; i < card->uid_len && off + 3U < size; i++) {
        off += (size_t)snprintf(out + off, size - off, "%s%02X",
                                (i == 0U) ? "" : ":", card->uid[i]);
    }
}

static void nfc_rebuild_dump(void)
{
    bool page_rows = (sg_card.type == NFC_CARD_NTAG ||
                      sg_card.type == NFC_CARD_ULTRALIGHT);
    uint16_t r;

    lv_obj_clean(sg_dump_list);
    if (sg_card.dump_rows == 0U) {
        lv_obj_t *empty = nfc_create_label(sg_dump_list, "No memory data",
                                           &lv_font_montserrat_14, NFC_MUTED);
        lv_obj_set_pos(empty, 4, 4);
        return;
    }
    for (r = 0; r < sg_card.dump_rows; r++) {
        lv_obj_t *row;
        char line[80];
        size_t off;

        if (sg_card.readable[r]) {
            uint8_t b;

            off = (size_t)snprintf(line, sizeof(line), "%02u: ",
                                   page_rows ? (unsigned)(r * 4U)
                                             : (unsigned)r);
            for (b = 0; b < 16U && off + 3U < sizeof(line); b++) {
                off += (size_t)snprintf(line + off, sizeof(line) - off,
                                        "%02X%s",
                                        sg_card.dump[r * 16U + b],
                                        (b == 7U) ? " " :
                                        ((b == 15U) ? "" : " "));
            }
        } else {
            snprintf(line, sizeof(line), "%02u:  -- unreadable --",
                     page_rows ? (unsigned)(r * 4U) : (unsigned)r);
        }
        row = nfc_create_label(sg_dump_list, line, &lv_font_montserrat_14,
                               sg_card.readable[r] ? NFC_WHITE : NFC_MUTED);
        /* Dump rows are *not* decorative — the dump list is a scrollable
         * container and swipes here must scroll the list, not switch screens.
         * Strip USER_1 (CLICKABLE stays so the row receives touch input that
         * the parent scroll container consumes). */
        lv_obj_clear_flag(row, LV_OBJ_FLAG_USER_1);
        lv_obj_set_width(row, 204);
        lv_label_set_long_mode(row, LV_LABEL_LONG_WRAP);
    }
}

static void nfc_clear_card_panel(void)
{
    lv_label_set_text(sg_type_label, "");
    lv_label_set_text(sg_uid_label, "");
    lv_label_set_text(sg_attr_label, "");
    lv_label_set_text(sg_ndef_label, "");
    lv_obj_clean(sg_dump_list);
}

static void nfc_forget_last_card(void)
{
    sg_card_present = false;
    sg_last_card_type = NFC_CARD_NONE;
    sg_last_uid_len = 0;
    memset(sg_last_uid, 0, sizeof(sg_last_uid));
}

static bool nfc_should_trigger_servo(const nfc_card_t *card)
{
    if (!sg_card_present ||
        sg_last_card_type != card->type ||
        sg_last_uid_len != card->uid_len ||
        memcmp(sg_last_uid, card->uid, card->uid_len) != 0) {
        sg_card_present = true;
        sg_last_card_type = card->type;
        sg_last_uid_len = card->uid_len;
        memcpy(sg_last_uid, card->uid, card->uid_len);
        return true;
    }
    return false;
}

static bool nfc_is_url(const char *s)
{
    return strncmp(s, "http://", 7) == 0 || strncmp(s, "https://", 8) == 0;
}

static void nfc_apply_read_result(void)
{
    char buf[64];

    if (sg_op_rt == OPRT_NOT_FOUND) {
        nfc_forget_last_card();
        nfc_set_status(NFC_CYAN, "No card");
        lv_label_set_text(sg_type_label, "No card detected");
        nfc_clear_card_panel();
        lv_label_set_text(sg_type_label, "No card detected");
        return;
    }
    if (sg_op_rt != OPRT_OK) {
        nfc_forget_last_card();
        nfc_set_status(NFC_RED, "Reader offline");
        nfc_clear_card_panel();
        lv_label_set_text(sg_type_label, "Check PN532 wiring");
        lv_label_set_text(sg_uid_label, pn532_i2c_diagnostic());
        return;
    }

    nfc_set_status(NFC_GREEN, "Detected");
    lv_label_set_text(sg_type_label, nfc_card_type_name(sg_card.type));

    nfc_uid_to_text(&sg_card, buf, sizeof(buf));
    lv_label_set_text(sg_uid_label, buf);

    if (sg_card.type == NFC_CARD_MIFARE_1K ||
        sg_card.type == NFC_CARD_MIFARE_4K ||
        sg_card.type == NFC_CARD_NTAG ||
        sg_card.type == NFC_CARD_ULTRALIGHT) {
        snprintf(buf, sizeof(buf), "ATQA %02X%02X SAK %02X | %u B",
                 sg_card.atqa[0], sg_card.atqa[1], sg_card.sak,
                 (unsigned)sg_card.size_bytes);
        lv_label_set_text(sg_attr_label, buf);
    } else {
        lv_label_set_text(sg_attr_label, "");
    }

    if (sg_card.content_id[0] != '\0') {
        lv_label_set_text(sg_ndef_caption, "SoundPola");
        lv_obj_set_style_text_color(sg_ndef_caption,
            lv_color_hex(NFC_GREEN), LV_PART_MAIN);
        lv_label_set_text(sg_ndef_label, sg_card.content_id);
        lv_obj_set_style_text_color(sg_ndef_label,
            lv_color_hex(NFC_GREEN), LV_PART_MAIN);
    } else if (sg_card.ndef_text[0] != '\0') {
        bool is_url = nfc_is_url(sg_card.ndef_text);

        lv_label_set_text(sg_ndef_caption, is_url ? "URL" : "NDEF text");
        lv_obj_set_style_text_color(sg_ndef_caption,
            lv_color_hex(is_url ? NFC_CYAN : NFC_MUTED), LV_PART_MAIN);
        lv_label_set_text(sg_ndef_label, sg_card.ndef_text);
        lv_obj_set_style_text_color(sg_ndef_label,
            lv_color_hex(is_url ? NFC_CYAN : NFC_WHITE), LV_PART_MAIN);
    } else {
        lv_label_set_text(sg_ndef_caption, "NDEF text");
        lv_obj_set_style_text_color(sg_ndef_caption,
            lv_color_hex(NFC_MUTED), LV_PART_MAIN);
        lv_label_set_text(sg_ndef_label, "");
    }
    nfc_rebuild_dump();
}

static void nfc_apply_write_result(void)
{
    if (sg_op_rt == OPRT_OK) {
        char msg[48];

        snprintf(msg, sizeof(msg), "Write OK (%s)",
                 nfc_card_type_name(sg_write_type));
        nfc_set_status(NFC_GREEN, msg);
    } else if (sg_op_rt == OPRT_NOT_FOUND) {
        nfc_set_status(NFC_CYAN, "No card detected");
    } else if (sg_op_rt == OPRT_NOT_SUPPORTED) {
        nfc_set_status(NFC_RED, "Card not writable");
    } else if (sg_op_rt == OPRT_INVALID_PARM) {
        nfc_set_status(NFC_RED, "Text too long");
    } else {
        nfc_set_status(NFC_RED, "Write failed (auth?)");
    }
}

static void nfc_try_autoplay(void)
{
    int idx;

    if (sg_card.content_id[0] == '\0') {
        return;
    }
    idx = playlist_store_find_by_id(sg_card.content_id);
    if (idx >= 0) {
        PR_NOTICE("nfc autoplay: %s -> idx %d (loop)", sg_card.content_id, idx);
        ui_home_play_index(idx, true);
        return;
    }
    /* Not in the local list yet — refresh once and retry when it lands. */
    snprintf(sg_pending_cid, sizeof(sg_pending_cid), "%s", sg_card.content_id);
    sg_pending_gen = playlist_store_generation();
    playlist_store_fetch();
    PR_NOTICE("nfc autoplay: %s not cached, fetching playlist", sg_pending_cid);
}

static void nfc_wcard_start_attempt(void)
{
    sg_wcard_attempts++;
    PR_NOTICE("nfc wcard: attempt %u/%u", (unsigned)sg_wcard_attempts,
              (unsigned)NFC_WCARD_MAX_ATTEMPTS);
    snprintf(sg_write_text, sizeof(sg_write_text), "%s", sg_wcard_url);
    sg_write_ndef_type = NFC_NDEF_URI;
    sg_write_requested = true;
}

static void nfc_wcard_retry_timer_cb(lv_timer_t *timer)
{
    lv_timer_pause(timer);
    if (sg_wcard_active) {
        nfc_wcard_start_attempt();
    }
}

bool app_nfc_write_card(const char *content_id)
{
    const char *server = playlist_store_server();

    if (sg_wcard_active) {
        return false;
    }
    if (content_id == NULL || content_id[0] == '\0' || server[0] == '\0') {
        return false;
    }
    snprintf(sg_wcard_url, sizeof(sg_wcard_url), "%s/c/%s", server, content_id);
    sg_wcard_active = true;
    sg_wcard_attempts = 0;
    nfc_wcard_start_attempt();
    return true;
}

static void nfc_wcard_finish(bool ok)
{
    sg_wcard_active = false;
    PR_NOTICE("nfc wcard: %s after %u attempts", ok ? "OK" : "FAILED",
              (unsigned)sg_wcard_attempts);
    ui_playlist_write_done(ok);
    ui_servo_eject();
}

static void nfc_wcard_handle_result(void)
{
    if (sg_op_rt == OPRT_OK) {
        nfc_wcard_finish(true);
        return;
    }
    if (sg_wcard_attempts >= NFC_WCARD_MAX_ATTEMPTS) {
        nfc_wcard_finish(false);
        return;
    }
    PR_NOTICE("nfc wcard: rt=%d, retry in 1s", sg_op_rt);
    if (sg_wcard_retry_timer == NULL) {
        sg_wcard_retry_timer = lv_timer_create(nfc_wcard_retry_timer_cb,
                                               1000, NULL);
    }
    lv_timer_set_period(sg_wcard_retry_timer, 1000);
    lv_timer_reset(sg_wcard_retry_timer);
    lv_timer_resume(sg_wcard_retry_timer);
}

static void nfc_ui_timer_cb(lv_timer_t *timer)
{
    bool is_write;
    bool trigger_servo = false;

    (void)timer;

    if (sg_pending_cid[0] != '\0' &&
        playlist_store_generation() != sg_pending_gen) {
        int idx = playlist_store_find_by_id(sg_pending_cid);

        if (idx >= 0) {
            PR_NOTICE("nfc autoplay (retry): %s -> idx %d", sg_pending_cid, idx);
            ui_home_play_index(idx, true);
        } else {
            PR_NOTICE("nfc autoplay: %s not in playlist, ignored",
                      sg_pending_cid);
        }
        sg_pending_cid[0] = '\0';
    }

    if (!sg_result_pending) {
        return;
    }
    is_write = sg_result_is_write;
    sg_result_pending = false;

    if (is_write) {
        if (sg_wcard_active) {
            nfc_wcard_handle_result();
            return;
        }
        nfc_apply_write_result();
        lv_obj_clear_state(sg_write_btn, LV_STATE_DISABLED);
        return;
    }

    /* Card-presence tracking + autoplay run regardless of which screen is
     * active; the read screen UI is only updated while it is visible. */
    if (sg_op_rt == OPRT_OK) {
        sg_miss_count = 0;
        trigger_servo = nfc_should_trigger_servo(&sg_card);
        if (trigger_servo) {
            PR_NOTICE("nfc trigger: cid='%s' (screen=%s)",
                      sg_card.content_id,
                      sg_auto_scan_enabled ? "nfc" : "other");
            if (sg_card.content_id[0] != '\0') {
                nfc_try_autoplay();
                ui_home_show_eject();
            } else if (!sg_wcard_active &&
                       (sg_card.type == NFC_CARD_NTAG ||
                        sg_card.type == NFC_CARD_ULTRALIGHT) &&
                       sg_card.dump_rows > 1U &&
                       sg_card.ndef_text[0] == '\0') {
                /* Blank writable card: open the song picker to write it. */
                PR_NOTICE("nfc blank card: open song picker");
                ui_playlist_enter_write_mode();
            }
        }
    } else if (sg_op_rt == OPRT_NOT_FOUND) {
        /* Debounce removal: one marginal poll must not re-arm the servo and
         * autoplay triggers for a card that is still on the reader. */
        if (sg_miss_count < 2U) {
            sg_miss_count++;
        }
        if (sg_miss_count >= 2U) {
            nfc_forget_last_card();
        }
    } else {
        nfc_forget_last_card();
    }

    if (!sg_auto_scan_enabled) {
        lv_obj_clear_state(sg_scan_btn, LV_STATE_DISABLED);
        return;
    }
    nfc_apply_read_result();
    if (trigger_servo) {
        nfc_set_status(NFC_GREEN, "Detected - autoplay");
    }
    lv_obj_clear_state(sg_scan_btn, LV_STATE_DISABLED);
}

static void nfc_request_scan(bool reset_ui)
{
    if (sg_mode_write || sg_scan_requested || sg_write_requested ||
        sg_result_pending) {
        return;
    }
    if (sg_auto_scan_enabled || reset_ui) {
        nfc_set_status(NFC_YELLOW, "Scanning...");
        if (reset_ui) {
            nfc_clear_card_panel();
            lv_label_set_text(sg_type_label, "Hold card near PN532");
        }
        lv_obj_add_state(sg_scan_btn, LV_STATE_DISABLED);
    }
    /* Background scans use the light single-poll path; a full read (memory
     * dump over the slow bit-banged bus) only happens on the NFC screen or
     * when a new card shows up. */
    sg_scan_full = sg_auto_scan_enabled || reset_ui;
    sg_scan_requested = true;
}

static void nfc_scan_button_cb(lv_event_t *event)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(event)) {
        return;
    }
    nfc_request_scan(true);
}

static void nfc_auto_scan_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    /* Resident 1 s polling: runs on every screen so a SoundPola card can
     * trigger playback anywhere. Write mode is guarded inside
     * nfc_request_scan(). */
    nfc_request_scan(false);
}

static const char *const sg_ndef_names[3] = {"TEXT", "URL", "APP"};
static const char *const sg_ndef_hints[3] = {
    "Type text to store on the card...",
    "https://example.com",
    "com.example.app",
};

static void nfc_update_type_buttons(void)
{
    uint8_t i;

    for (i = 0; i < 3U; i++) {
        bool sel = (i == sg_ndef_type);

        lv_obj_set_style_bg_color(sg_type_btns[i],
            lv_color_hex(sel ? NFC_CYAN : NFC_CARD), LV_PART_MAIN);
        lv_obj_set_style_text_color(lv_obj_get_child(sg_type_btns[i], 0),
            lv_color_hex(sel ? NFC_BG_TOP : NFC_MUTED), LV_PART_MAIN);
    }
}

static void nfc_type_button_cb(lv_event_t *event)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(event)) {
        return;
    }
    sg_ndef_type = (uint8_t)(uintptr_t)lv_event_get_user_data(event);
    lv_textarea_set_placeholder_text(sg_textarea,
                                     sg_ndef_hints[sg_ndef_type]);
    nfc_update_type_buttons();
}

static void nfc_write_button_cb(lv_event_t *event)
{
    const char *text;
    size_t len;

    if (LV_EVENT_CLICKED != lv_event_get_code(event) || sg_write_requested) {
        return;
    }
    text = lv_textarea_get_text(sg_textarea);
    len = strlen(text);
    if (len == 0U) {
        nfc_set_status(NFC_YELLOW, "Enter text first");
        return;
    }
    if (len > WRITE_TEXT_MAX) {
        len = WRITE_TEXT_MAX;
    }
    memcpy(sg_write_text, text, len);
    sg_write_text[len] = '\0';
    sg_write_ndef_type = sg_ndef_type;

    nfc_set_status(NFC_YELLOW, "Hold card near PN532");
    lv_obj_add_state(sg_write_btn, LV_STATE_DISABLED);
    sg_write_requested = true;
}

static void nfc_mode_button_cb(lv_event_t *event)
{
    lv_obj_t *target = lv_event_get_user_data(event);

    if (LV_EVENT_CLICKED != lv_event_get_code(event)) {
        return;
    }
    if (target == sg_screen_write) {
        sg_auto_scan_enabled = false;
        sg_scan_requested = false;
        sg_mode_write = true;
        lv_label_set_text(sg_w_status_label, "Enter text, then WRITE");
        lv_screen_load_anim(sg_screen_write, LV_SCR_LOAD_ANIM_FADE_IN,
                            180, 0, false);
    } else {
        sg_mode_write = false;
        lv_screen_load_anim(sg_screen_read, LV_SCR_LOAD_ANIM_FADE_IN,
                            180, 0, false);
    }
}

static void nfc_back_button_cb(lv_event_t *event)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(event)) {
        return;
    }
    sg_auto_scan_enabled = false;
    sg_scan_requested = false;
    ui_nav_go_anim(UI_SCR_HOME, LV_SCR_LOAD_ANIM_MOVE_TOP, 250);
}

static void nfc_read_screen_event_cb(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);

    if (code == LV_EVENT_SCREEN_UNLOAD_START ||
        code == LV_EVENT_SCREEN_UNLOADED) {
        sg_auto_scan_enabled = false;
        sg_scan_requested = false;
        return;
    }
    if (code != LV_EVENT_SCREEN_LOADED) {
        return;
    }

    /* Reset state when returning from a deeper screen or the write sub-screen */
    sg_mode_write = false;
    sg_scan_requested = false;
    sg_write_requested = false;
    sg_result_pending = false;
    sg_result_is_write = false;
    sg_auto_scan_enabled = true;
    lv_obj_clear_state(sg_scan_btn, LV_STATE_DISABLED);
    nfc_set_status(NFC_YELLOW, "Auto scanning...");
    nfc_clear_card_panel();
    lv_label_set_text(sg_type_label, "Hold card near PN532");
    nfc_request_scan(false);
}

static void nfc_build_topbar(lv_obj_t *screen, bool is_write)
{
    lv_obj_t *label;
    lv_obj_t *btn;
    lv_obj_t *btn_label;

    btn = lv_button_create(screen);
    lv_obj_set_pos(btn, 4, 4);
    lv_obj_set_size(btn, 36, 28);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(NFC_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, nfc_back_button_cb, LV_EVENT_CLICKED, NULL);
    label = nfc_create_label(btn, LV_SYMBOL_LEFT, &lv_font_montserrat_14,
                             NFC_WHITE);
    lv_obj_center(label);

    label = nfc_create_label(screen, "NFC Reader/Writer",
                             &lv_font_montserrat_20, NFC_WHITE);
    lv_obj_set_pos(label, 48, 8);

    btn = lv_button_create(screen);
    lv_obj_set_pos(btn, 288, 4);
    lv_obj_set_size(btn, 56, 28);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn,
        lv_color_hex(is_write ? NFC_CARD : NFC_CYAN), LV_PART_MAIN);
    lv_obj_add_event_cb(btn, nfc_mode_button_cb, LV_EVENT_CLICKED,
                        sg_screen_read);
    btn_label = nfc_create_label(btn, "READ", &lv_font_montserrat_14,
                                 is_write ? NFC_MUTED : NFC_BG_TOP);
    lv_obj_center(btn_label);

    btn = lv_button_create(screen);
    lv_obj_set_pos(btn, 352, 4);
    lv_obj_set_size(btn, 64, 28);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn,
        lv_color_hex(is_write ? NFC_CYAN : NFC_CARD), LV_PART_MAIN);
    lv_obj_add_event_cb(btn, nfc_mode_button_cb, LV_EVENT_CLICKED,
                        sg_screen_write);
    btn_label = nfc_create_label(btn, "WRITE", &lv_font_montserrat_14,
                                 is_write ? NFC_BG_TOP : NFC_MUTED);
    lv_obj_center(btn_label);
}

static void nfc_style_screen(lv_obj_t *screen)
{
    lv_obj_remove_style_all(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(NFC_BG_TOP), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(NFC_BG_BOTTOM),
                                   LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    /* The screen itself must be clickable so that presses on empty background
     * areas are recorded by the press-target recorder and produce a gesture
     * event we can match in the screen-level gesture callback. */
    lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICKABLE);
}

static void nfc_build_read_screen(void)
{
    lv_obj_t *left;
    lv_obj_t *right;
    lv_obj_t *label;

    nfc_style_screen(sg_screen_read);
    nfc_build_topbar(sg_screen_read, false);

    /* status dot + label in top bar */
    sg_status_dot = lv_obj_create(sg_screen_read);
    nfc_remove_default_style(sg_status_dot);
    lv_obj_set_size(sg_status_dot, 10, 10);
    lv_obj_set_style_radius(sg_status_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_status_dot, lv_color_hex(NFC_YELLOW),
                              LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_status_dot, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_pos(sg_status_dot, 428, 13);
    lv_obj_add_flag(sg_status_dot, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(sg_status_dot, LV_OBJ_FLAG_USER_1);
    sg_status_label = nfc_create_label(sg_screen_read, "Ready",
                                       &lv_font_montserrat_14, NFC_MUTED);
    lv_obj_set_pos(sg_status_label, 442, 9);

    /* left panel: card identity */
    left = nfc_create_panel(sg_screen_read, 8, 40, 226, 224);

    sg_type_label = nfc_create_label(left, "Hold card near PN532",
                                     &lv_font_montserrat_20, NFC_WHITE);
    lv_obj_set_pos(sg_type_label, 14, 12);
    lv_obj_set_width(sg_type_label, 200);
    lv_label_set_long_mode(sg_type_label, LV_LABEL_LONG_WRAP);

    sg_uid_label = nfc_create_label(left, "", &lv_font_montserrat_14,
                                    NFC_CYAN);
    lv_obj_set_pos(sg_uid_label, 14, 66);
    lv_obj_set_width(sg_uid_label, 200);
    lv_label_set_long_mode(sg_uid_label, LV_LABEL_LONG_WRAP);

    sg_attr_label = nfc_create_label(left, "", &lv_font_montserrat_14,
                                     NFC_MUTED);
    lv_obj_set_pos(sg_attr_label, 14, 92);
    lv_obj_set_width(sg_attr_label, 200);
    lv_label_set_long_mode(sg_attr_label, LV_LABEL_LONG_WRAP);

    sg_ndef_caption = nfc_create_label(left, "NDEF text",
                                       &lv_font_montserrat_14, NFC_MUTED);
    lv_obj_set_pos(sg_ndef_caption, 14, 140);
    sg_ndef_label = nfc_create_label(left, "", &lv_font_montserrat_14,
                                     NFC_WHITE);
    lv_obj_set_pos(sg_ndef_label, 14, 162);
    lv_obj_set_width(sg_ndef_label, 200);
    lv_label_set_long_mode(sg_ndef_label, LV_LABEL_LONG_WRAP);

    /* right panel: memory dump */
    right = nfc_create_panel(sg_screen_read, 242, 40, 230, 224);

    label = nfc_create_label(right, "Memory", &lv_font_montserrat_14,
                             NFC_CYAN);
    lv_obj_set_pos(label, 14, 10);

    sg_dump_list = lv_obj_create(right);
    nfc_remove_default_style(sg_dump_list);
    lv_obj_set_pos(sg_dump_list, 12, 34);
    lv_obj_set_size(sg_dump_list, 214, 182);
    lv_obj_set_style_bg_opa(sg_dump_list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_flow(sg_dump_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sg_dump_list, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(sg_dump_list, LV_SCROLLBAR_MODE_AUTO);

    /* bottom scan button */
    sg_scan_btn = lv_button_create(sg_screen_read);
    lv_obj_set_pos(sg_scan_btn, 8, 272);
    lv_obj_set_size(sg_scan_btn, 464, 40);
    lv_obj_set_style_radius(sg_scan_btn, 14, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_scan_btn, lv_color_hex(NFC_CYAN),
                              LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(sg_scan_btn, lv_color_hex(0x3992FF),
                                   LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(sg_scan_btn, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(sg_scan_btn, 12, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(sg_scan_btn, lv_color_hex(NFC_CYAN),
                                  LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(sg_scan_btn, LV_OPA_30, LV_PART_MAIN);
    lv_obj_add_event_cb(sg_scan_btn, nfc_scan_button_cb, LV_EVENT_CLICKED,
                        NULL);
    label = nfc_create_label(sg_scan_btn, "AUTO SCAN", &lv_font_montserrat_20,
                             NFC_BG_TOP);
    lv_obj_center(label);
}

static void nfc_build_write_screen(void)
{
    uint8_t i;

    nfc_style_screen(sg_screen_write);
    nfc_build_topbar(sg_screen_write, true);

    /* record type selector */
    for (i = 0; i < 3U; i++) {
        lv_obj_t *btn = lv_button_create(sg_screen_write);
        lv_obj_t *lbl;

        lv_obj_set_pos(btn, 8 + (int32_t)i * 88, 42);
        lv_obj_set_size(btn, 80, 28);
        lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
        lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x5D7891),
                                      LV_PART_MAIN);
        lv_obj_add_event_cb(btn, nfc_type_button_cb, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)i);
        lbl = nfc_create_label(btn, sg_ndef_names[i], &lv_font_montserrat_14,
                               NFC_MUTED);
        lv_obj_center(lbl);
        sg_type_btns[i] = btn;
    }

    sg_textarea = lv_textarea_create(sg_screen_write);
    lv_obj_set_pos(sg_textarea, 8, 76);
    lv_obj_set_size(sg_textarea, 464, 40);
    lv_textarea_set_one_line(sg_textarea, true);
    lv_textarea_set_max_length(sg_textarea, WRITE_TEXT_MAX);
    lv_textarea_set_placeholder_text(sg_textarea, sg_ndef_hints[0]);
    lv_obj_set_style_text_font(sg_textarea, &lv_font_montserrat_14,
                               LV_PART_MAIN);
    lv_obj_set_style_radius(sg_textarea, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_textarea, lv_color_hex(NFC_CARD),
                              LV_PART_MAIN);
    lv_obj_set_style_text_color(sg_textarea, lv_color_hex(NFC_WHITE),
                                LV_PART_MAIN);
    lv_obj_set_style_border_width(sg_textarea, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(sg_textarea, lv_color_hex(0x5D7891),
                                  LV_PART_MAIN);

    /* lv_keyboard defaults to BOTTOM_MID alignment; set_align overrides it,
     * otherwise a pos offset pushes the lower rows off screen */
    sg_keyboard = lv_keyboard_create(sg_screen_write);
    lv_obj_set_size(sg_keyboard, 464, 150);
    lv_obj_set_align(sg_keyboard, LV_ALIGN_TOP_LEFT);
    lv_obj_set_pos(sg_keyboard, 8, 122);
    lv_keyboard_set_textarea(sg_keyboard, sg_textarea);
    lv_obj_set_style_bg_color(sg_keyboard, lv_color_hex(NFC_BG_BOTTOM),
                              LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_keyboard, lv_color_hex(NFC_CARD),
                              LV_PART_ITEMS);
    lv_obj_set_style_text_color(sg_keyboard, lv_color_hex(NFC_WHITE),
                                LV_PART_ITEMS);
    lv_obj_set_style_text_font(sg_keyboard, &lv_font_montserrat_14,
                               LV_PART_ITEMS);

    sg_w_status_label = nfc_create_label(sg_screen_write,
                                         "Enter text, then WRITE",
                                         &lv_font_montserrat_14, NFC_MUTED);
    lv_obj_set_pos(sg_w_status_label, 12, 284);

    sg_write_btn = lv_button_create(sg_screen_write);
    lv_obj_set_pos(sg_write_btn, 330, 276);
    lv_obj_set_size(sg_write_btn, 142, 36);
    lv_obj_set_style_radius(sg_write_btn, 14, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_write_btn, lv_color_hex(NFC_GREEN),
                              LV_PART_MAIN);
    lv_obj_set_style_shadow_width(sg_write_btn, 12, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(sg_write_btn, lv_color_hex(NFC_GREEN),
                                  LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(sg_write_btn, LV_OPA_30, LV_PART_MAIN);
    lv_obj_add_event_cb(sg_write_btn, nfc_write_button_cb, LV_EVENT_CLICKED,
                        NULL);
    {
        lv_obj_t *label = nfc_create_label(sg_write_btn, "WRITE",
                                           &lv_font_montserrat_20,
                                           NFC_BG_TOP);
        lv_obj_center(label);
    }
    nfc_update_type_buttons();
}

static bool nfc_same_as_last(const nfc_card_t *card)
{
    return sg_card_present &&
           sg_last_uid_len == card->uid_len &&
           memcmp(sg_last_uid, card->uid, card->uid_len) == 0;
}

static void nfc_worker(void *arg)
{
    (void)arg;

    while (1) {
        if (sg_scan_requested) {
            bool full = sg_scan_full;

            memset(&sg_card, 0, sizeof(sg_card));
            if (full) {
                sg_op_rt = nfc_ops_read_card(&sg_card);
            } else {
                sg_op_rt = nfc_ops_detect(&sg_card);
                if (sg_op_rt == OPRT_OK && nfc_same_as_last(&sg_card)) {
                    /* Same card still resting on the reader: presence is
                     * confirmed, skip the expensive dump and post nothing. */
                    sg_miss_count = 0;
                    sg_scan_requested = false;
                    continue;
                }
                if (sg_op_rt == OPRT_OK) {
                    PR_NOTICE("bg detect: new card uid_len=%u, full read",
                              sg_card.uid_len);
                    sg_op_rt = nfc_ops_read_detected(&sg_card);
                    PR_NOTICE("bg read rt=%d cid='%s'", sg_op_rt,
                              sg_card.content_id);
                }
            }
            if (sg_op_rt == OPRT_OK) {
                PR_NOTICE("read card type=%d rows=%u ndef='%s'",
                          sg_card.type, sg_card.dump_rows, sg_card.ndef_text);
            } else if (sg_op_rt != OPRT_NOT_FOUND || full) {
                PR_NOTICE("read card rt=%d", sg_op_rt);
            }
            sg_scan_requested = false;
            sg_result_is_write = false;
            sg_result_pending = true;
        } else if (sg_write_requested) {
            char text[WRITE_TEXT_MAX + 1];
            nfc_card_type_e wt = NFC_CARD_NONE;

            memcpy(text, sg_write_text, sizeof(text));
            sg_op_rt = nfc_ops_write_record((nfc_ndef_type_e)sg_write_ndef_type,
                                            text, &wt);
            sg_write_type = wt;
            PR_NOTICE("write text rt=%d type=%d", sg_op_rt, wt);
            sg_write_requested = false;
            sg_result_is_write = true;
            sg_result_pending = true;
        } else {
            tal_system_sleep(30);
        }
    }
}

OPERATE_RET app_nfc_init(void)
{
    THREAD_CFG_T thread_cfg = {
        .stackDepth = 1024 * 8,
        .priority = THREAD_PRIO_2,
        .thrdname = "pn532_worker",
    };
    OPERATE_RET rt;

    PR_NOTICE("starting nfc reader/writer");
    sg_scan_requested = false;
    sg_write_requested = false;
    sg_result_pending = false;
    sg_result_is_write = false;
    sg_auto_scan_enabled = false;
    sg_mode_write = false;
    nfc_forget_last_card();

    sg_screen_read = lv_obj_create(NULL);
    sg_screen_write = lv_obj_create(NULL);
    nfc_build_read_screen();
    nfc_build_write_screen();

    ui_nav_register(UI_SCR_NFC, sg_screen_read);
    ui_nav_attach_gesture(sg_screen_read);
    lv_obj_add_event_cb(sg_screen_read, nfc_read_screen_event_cb,
                        LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(sg_screen_read, nfc_read_screen_event_cb,
                        LV_EVENT_SCREEN_UNLOAD_START, NULL);
    lv_obj_add_event_cb(sg_screen_read, nfc_read_screen_event_cb,
                        LV_EVENT_SCREEN_UNLOADED, NULL);
    /* The write sub-screen is not registered with ui_nav (the READ screen is
     * the entry point for UI_SCR_NFC), but its gesture handler still needs
     * the press-target recorder and the release-reset hook. */
    ui_nav_attach_press(sg_screen_write);
    lv_obj_add_event_cb(sg_screen_write, ui_nav_go_home_on_bottom_cb,
                        LV_EVENT_GESTURE, NULL);

    lv_timer_create(nfc_ui_timer_cb, 100, NULL);
    lv_timer_create(nfc_auto_scan_timer_cb, NFC_AUTO_SCAN_MS, NULL);

    rt = tal_thread_create_and_start(&sg_worker, NULL, NULL,
                                     nfc_worker, NULL, &thread_cfg);
    if (OPRT_OK != rt) {
        PR_ERR("NFC worker thread create failed: %d", rt);
    }
    return rt;
}
