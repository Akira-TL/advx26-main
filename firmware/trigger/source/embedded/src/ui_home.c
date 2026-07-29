#include <stdio.h>

#include "tal_api.h"
#include "lvgl.h"

#include "ui_home.h"
#include "ui_nav.h"
#include "ui_servo.h"
#include "playlist_store.h"
#include "master_link.h"

LV_FONT_DECLARE(font_cn_16);

#define HOME_BLACK   0x000000
#define HOME_WHITE   0xFFFFFF
#define HOME_GRAY    0x888888
#define HOME_ACCENT  0x48D8FF

static lv_obj_t *sg_scr;
static lv_obj_t *sg_title_label;
static lv_obj_t *sg_artist_label;
static lv_obj_t *sg_slider;
static lv_obj_t *sg_time_cur;
static lv_obj_t *sg_time_total;
static lv_obj_t *sg_play_btn;
static lv_obj_t *sg_eject_btn;
static lv_obj_t *sg_play_label;

static int sg_song_idx;
static uint16_t sg_position;
static bool sg_playing;
static bool sg_dragging;
static bool sg_loop;
static uint32_t sg_seen_generation;

/* Two-phase remote playback: load -> (slave ready) -> start. */
typedef enum {
    HOME_PHASE_IDLE = 0,
    HOME_PHASE_BUFFERING,   /* load sent, waiting slave ready */
    HOME_PHASE_PLAYING,
    HOME_PHASE_PAUSED,
} home_phase_e;

static home_phase_e sg_phase;
static bool sg_local_mode;  /* slave absent: simulate progress locally */
static uint8_t sg_tick;     /* 200ms tick divider for local mode */

static void home_format_time(uint16_t sec, char *out, size_t size)
{
    snprintf(out, size, "%u:%02u", (unsigned)(sec / 60U), (unsigned)(sec % 60U));
}

static uint16_t home_cur_duration(void)
{
    const plist_item_t *pi = playlist_store_get(sg_song_idx);
    if (pi == NULL) return 0;
    return (uint16_t)(pi->duration_ms / 1000U);
}

static void home_show_empty(const char *text)
{
    sg_playing = false;
    sg_position = 0;
    sg_phase = HOME_PHASE_IDLE;
    lv_label_set_text(sg_title_label, text);
    lv_label_set_text(sg_artist_label, "");
    lv_slider_set_range(sg_slider, 0, 1);
    lv_slider_set_value(sg_slider, 0, LV_ANIM_OFF);
    lv_label_set_text(sg_time_cur, "0:00");
    lv_label_set_text(sg_time_total, "0:00");
    lv_label_set_text(sg_play_label, LV_SYMBOL_PLAY);
    lv_obj_add_state(sg_play_btn, LV_STATE_DISABLED);
}

static void home_update_labels(void)
{
    char buf[32];
    int count = playlist_store_count();

    if (count == 0) {
        switch (playlist_store_status()) {
            case PLIST_STORE_LOADING:
                home_show_empty("加载中…");
                break;
            case PLIST_STORE_NOT_PAIRED:
            case PLIST_STORE_TOKEN_EXPIRED:
                home_show_empty("请先扫码配对");
                break;
            case PLIST_STORE_NET_ERROR:
                home_show_empty("网络错误");
                break;
            default:
                home_show_empty("暂无声音");
                break;
        }
        return;
    }

    if (sg_song_idx >= count) sg_song_idx = 0;
    const plist_item_t *pi = playlist_store_get(sg_song_idx);
    uint16_t duration = home_cur_duration();

    lv_obj_clear_state(sg_play_btn, LV_STATE_DISABLED);
    lv_label_set_text(sg_title_label, pi->label);
    if (pi->state == PLIST_ITEM_PROCESSING) {
        snprintf(buf, sizeof(buf), "%d/%d · 处理中", sg_song_idx + 1, count);
    } else if (pi->state == PLIST_ITEM_FAILED) {
        snprintf(buf, sizeof(buf), "%d/%d · 失败", sg_song_idx + 1, count);
    } else {
        snprintf(buf, sizeof(buf), "%d/%d", sg_song_idx + 1, count);
    }
    lv_label_set_text(sg_artist_label, buf);
    lv_slider_set_range(sg_slider, 0, duration > 0 ? duration : 1);
    lv_slider_set_value(sg_slider, sg_position, LV_ANIM_OFF);
    home_format_time(sg_position, buf, sizeof(buf));
    lv_label_set_text(sg_time_cur, buf);
    home_format_time(duration, buf, sizeof(buf));
    lv_label_set_text(sg_time_total, buf);
}

static void home_update_play_btn(void)
{
    lv_label_set_text(sg_play_label, sg_playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
}

static void home_send_load(void)
{
    if (!sg_playing) return;
    if (playlist_store_get(sg_song_idx) == NULL) return;
    sg_tick = 0;
    if (playlist_send_load(sg_song_idx)) {
        master_link_note_load();
        sg_local_mode = false;
        sg_phase = HOME_PHASE_BUFFERING;
        lv_label_set_text(sg_artist_label, "缓冲中…");
    } else {
        sg_local_mode = true;
        sg_phase = HOME_PHASE_PLAYING;
        lv_label_set_text(sg_artist_label, "未连接播放板，请到 COMM 页连接");
    }
}

static void home_next_song(void)
{
    int count = playlist_store_count();
    if (count == 0) return;
    sg_song_idx = (sg_song_idx + 1) % count;
    sg_position = 0;
    home_update_labels();
    home_send_load();
}

static void home_prev_song(void)
{
    int count = playlist_store_count();
    if (count == 0) return;
    if (sg_position > 3U) {
        sg_position = 0;
    } else {
        sg_song_idx = (sg_song_idx + count - 1) % count;
        sg_position = 0;
    }
    home_update_labels();
    home_send_load();
}

static void home_finish_song(void)
{
    if (sg_loop) {
        sg_position = 0;
        home_update_labels();
        home_send_load();
    } else {
        home_next_song();
    }
}

static void home_timer_cb(lv_timer_t *timer)
{
    char buf[16];
    uint16_t duration;
    slave_status_t sl;

    (void)timer;

    uint32_t gen = playlist_store_generation();
    if (gen != sg_seen_generation) {
        sg_seen_generation = gen;
        home_update_labels();
        home_update_play_btn();
    }

    if (playlist_store_count() == 0) {
        return;
    }

    sl = master_link_slave_status();

    if (sg_phase == HOME_PHASE_BUFFERING) {
        if (!master_link_connected()) {
            sg_local_mode = true;
            sg_phase = HOME_PHASE_PLAYING;
            lv_label_set_text(sg_artist_label, "连接断开，仅本机模拟");
        } else if (sl.state == SLAVE_STATE_ERROR) {
            sg_phase = HOME_PHASE_IDLE;
            sg_playing = false;
            home_update_play_btn();
            lv_label_set_text(sg_artist_label, "从板加载失败");
        } else if (sl.state == SLAVE_STATE_READY ||
                   sl.state == SLAVE_STATE_PAUSED) {
            /* PAUSED also counts: after load the engine parks in PAUSED and
             * the 1s periodic report may overwrite the one-shot ready line. */
            master_link_send_start();
            sg_phase = HOME_PHASE_PLAYING;
            home_update_labels();
        }
        return;
    }

    if (sg_phase != HOME_PHASE_PLAYING || !sg_playing || sg_dragging) {
        return;
    }

    if (!sg_local_mode) {
        if (!master_link_connected()) {
            sg_local_mode = true;
            lv_label_set_text(sg_artist_label, "连接断开，仅本机模拟");
            return;
        }
        if (sl.state == SLAVE_STATE_COMPLETED) {
            home_finish_song();
            return;
        }
        if (sl.state == SLAVE_STATE_ERROR) {
            sg_phase = HOME_PHASE_IDLE;
            sg_playing = false;
            home_update_play_btn();
            lv_label_set_text(sg_artist_label, "从板播放出错");
            return;
        }
        if (sl.state != SLAVE_STATE_PLAYING) {
            return; /* start in flight / slave buffering */
        }
        duration = (uint16_t)(sl.duration_ms / 1000U);
        if (duration == 0) {
            duration = home_cur_duration();
        }
        if (duration == 0) {
            return;
        }
        sg_position = (uint16_t)(sl.position_ms / 1000U);
        if (sg_position > duration) sg_position = duration;
        lv_slider_set_range(sg_slider, 0, duration);
        lv_slider_set_value(sg_slider, sg_position, LV_ANIM_OFF);
        home_format_time(sg_position, buf, sizeof(buf));
        lv_label_set_text(sg_time_cur, buf);
        home_format_time(duration, buf, sizeof(buf));
        lv_label_set_text(sg_time_total, buf);
        return;
    }

    /* Local fallback: simulate 1s progress (timer runs at 200ms). */
    if (++sg_tick < 5) {
        return;
    }
    sg_tick = 0;
    duration = home_cur_duration();
    if (duration == 0) {
        return;
    }
    sg_position++;
    if (sg_position >= duration) {
        home_finish_song();
        return;
    }
    lv_slider_set_value(sg_slider, sg_position, LV_ANIM_OFF);
    home_format_time(sg_position, buf, sizeof(buf));
    lv_label_set_text(sg_time_cur, buf);
}

static void home_slider_pressed_cb(lv_event_t *e)
{
    (void)e;
    sg_dragging = true;
}

static void home_slider_released_cb(lv_event_t *e)
{
    char buf[16];

    (void)e;
    sg_dragging = false;
    sg_position = (uint16_t)lv_slider_get_value(sg_slider);
    home_format_time(sg_position, buf, sizeof(buf));
    lv_label_set_text(sg_time_cur, buf);
}

static void home_play_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) {
        return;
    }
    if (playlist_store_count() == 0) {
        return;
    }
    switch (sg_phase) {
    case HOME_PHASE_PLAYING:
        if (!sg_local_mode) {
            master_link_send_pause();
        }
        sg_phase = HOME_PHASE_PAUSED;
        sg_playing = false;
        break;
    case HOME_PHASE_PAUSED:
        if (!sg_local_mode) {
            master_link_send_resume();
        }
        sg_phase = HOME_PHASE_PLAYING;
        sg_playing = true;
        break;
    case HOME_PHASE_BUFFERING:
        master_link_send_stop();
        sg_phase = HOME_PHASE_IDLE;
        sg_playing = false;
        home_update_labels();
        break;
    default:
        sg_playing = true;
        home_send_load();
        break;
    }
    home_update_play_btn();
}

static void home_drop_paused(void)
{
    if (sg_phase == HOME_PHASE_PAUSED) {
        if (!sg_local_mode) {
            master_link_send_stop();
        }
        sg_phase = HOME_PHASE_IDLE;
        sg_playing = false;
        home_update_play_btn();
    }
}

static void home_prev_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) {
        return;
    }
    sg_loop = false;
    home_drop_paused();
    home_prev_song();
}

static void home_next_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) {
        return;
    }
    sg_loop = false;
    home_drop_paused();
    home_next_song();
}

static void home_eject_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) {
        return;
    }
    ui_servo_eject();
    lv_obj_add_flag(sg_eject_btn, LV_OBJ_FLAG_HIDDEN);
}

static void home_loaded_cb(lv_event_t *e)
{
    (void)e;
    if (playlist_store_count() == 0 &&
        playlist_store_status() != PLIST_STORE_LOADING) {
        playlist_store_fetch();
    }
    home_update_labels();
    home_update_play_btn();
}

void ui_home_play_index(int idx, bool loop)
{
    if (playlist_store_get(idx) == NULL) {
        return;
    }
    sg_song_idx = idx;
    sg_position = 0;
    sg_playing = true;
    sg_loop = loop;
    home_update_labels();
    home_update_play_btn();
    home_send_load();
}

void ui_home_show_eject(void)
{
    if (sg_eject_btn != NULL) {
        lv_obj_clear_flag(sg_eject_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

static lv_obj_t *home_create_label(lv_obj_t *parent, const char *text,
                                   const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    /* Decorative label — let background swipes pass through (USER_1 wins
     * over CLICKABLE in the gesture filter). The default PRESSED-state
     * style is empty so the label does not flash a "pressed" color when
     * the user is swiping across it. */
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(label, LV_OBJ_FLAG_USER_1);
    return label;
}

static lv_obj_t *home_create_ctrl_btn(lv_obj_t *parent, int32_t x, int32_t w,
                                      const char *symbol, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_t *lbl;

    lv_obj_set_pos(btn, x, 240);
    lv_obj_set_size(btn, w, 44);
    lv_obj_set_style_radius(btn, 22, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, symbol);
    lv_obj_set_style_text_color(lbl, lv_color_hex(HOME_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_center(lbl);
    return btn;
}

void ui_home_init(void)
{
    lv_obj_t *hint;

    sg_scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(sg_scr);
    lv_obj_set_style_bg_color(sg_scr, lv_color_hex(HOME_BLACK), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(sg_scr, LV_OBJ_FLAG_SCROLLABLE);
    /* Screen must be clickable so presses on empty background areas are
     * recorded by the press-target recorder and the screen-level gesture
     * handler can fire. */
    lv_obj_add_flag(sg_scr, LV_OBJ_FLAG_CLICKABLE);

    sg_title_label = home_create_label(sg_scr, "", &font_cn_16, HOME_WHITE);
    lv_obj_set_pos(sg_title_label, 0, 60);
    lv_obj_set_width(sg_title_label, 480);
    lv_obj_set_style_text_align(sg_title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_long_mode(sg_title_label, LV_LABEL_LONG_DOT);

    sg_artist_label = home_create_label(sg_scr, "", &font_cn_16, HOME_GRAY);
    lv_obj_set_pos(sg_artist_label, 0, 92);
    lv_obj_set_width(sg_artist_label, 480);
    lv_obj_set_style_text_align(sg_artist_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    sg_slider = lv_slider_create(sg_scr);
    lv_obj_set_pos(sg_slider, 40, 180);
    lv_obj_set_size(sg_slider, 400, 6);
    lv_obj_set_style_bg_color(sg_slider, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sg_slider, lv_color_hex(HOME_ACCENT), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(sg_slider, lv_color_hex(HOME_WHITE), LV_PART_KNOB);
    lv_obj_set_style_pad_all(sg_slider, 6, LV_PART_KNOB);
    lv_obj_add_event_cb(sg_slider, home_slider_pressed_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(sg_slider, home_slider_released_cb, LV_EVENT_RELEASED, NULL);

    sg_time_cur = home_create_label(sg_scr, "0:00", &lv_font_montserrat_14, HOME_GRAY);
    lv_obj_set_pos(sg_time_cur, 40, 196);

    sg_time_total = home_create_label(sg_scr, "0:00", &lv_font_montserrat_14, HOME_GRAY);
    lv_obj_set_pos(sg_time_total, 400, 196);

    home_create_ctrl_btn(sg_scr, 140, 52, LV_SYMBOL_PREV, home_prev_cb);
    sg_play_btn = home_create_ctrl_btn(sg_scr, 210, 60, LV_SYMBOL_PLAY, home_play_cb);
    sg_play_label = lv_obj_get_child(sg_play_btn, 0);
    home_create_ctrl_btn(sg_scr, 290, 52, LV_SYMBOL_NEXT, home_next_cb);
    sg_eject_btn = home_create_ctrl_btn(sg_scr, 360, 52, LV_SYMBOL_EJECT,
                                        home_eject_cb);
    lv_obj_add_flag(sg_eject_btn, LV_OBJ_FLAG_HIDDEN);

    hint = home_create_label(sg_scr,
                             "Swipe UP: Comm  DOWN: NFC  LEFT: Servo  RIGHT: WiFi",
                             &lv_font_montserrat_14, 0x555555);
    lv_obj_set_pos(hint, 0, 300);
    lv_obj_set_width(hint, 480);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    {
        lv_obj_t *ver = home_create_label(sg_scr, "v0.4", &lv_font_montserrat_14,
                                          0x444444);
        lv_obj_set_pos(ver, 440, 300);
    }

    sg_song_idx = 0;
    sg_position = 0;
    sg_playing = false;
    sg_dragging = false;
    home_update_labels();
    home_update_play_btn();

    lv_obj_add_event_cb(sg_scr, home_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);

    lv_timer_create(home_timer_cb, 200, NULL);

    ui_nav_register(UI_SCR_HOME, sg_scr);
    ui_nav_attach_gesture(sg_scr);
}
