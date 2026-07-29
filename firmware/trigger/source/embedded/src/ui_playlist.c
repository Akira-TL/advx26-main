#include <stdio.h>
#include <string.h>

#include "tal_api.h"
#include "lvgl.h"

#include "ui_playlist.h"
#include "ui_nav.h"
#include "playlist_store.h"
#include "app_nfc.h"

LV_FONT_DECLARE(font_cn_16);

#define PLIST_BG        0x000000
#define PLIST_CARD      0x111111
#define PLIST_WHITE     0xF5F5F5
#define PLIST_MUTED     0x8A8A8A
#define PLIST_YELLOW    0xFFD166
#define PLIST_RED       0xFF6B7A
#define PLIST_GREEN     0x48E0A4

static lv_obj_t *sg_scr;
static lv_obj_t *sg_status_label;
static lv_obj_t *sg_list;

static uint32_t sg_seen_generation;

/* Blank-card write flow: 0 normal, 1 picking a song, 2 writing the card. */
static uint8_t sg_write_state;
static lv_timer_t *sg_back_home_timer;

static void plist_back_home_timer_cb(lv_timer_t *timer)
{
    lv_timer_pause(timer);
    if (lv_screen_active() == sg_scr) {
        ui_nav_go_anim(UI_SCR_HOME, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250);
    }
}

static void plist_format_duration(uint32_t ms, char *out, size_t size)
{
    uint32_t total_sec = ms / 1000U;
    snprintf(out, size, "%u:%02u", (unsigned)(total_sec / 60U),
             (unsigned)(total_sec % 60U));
}

static void plist_row_cb(lv_event_t *e)
{
    int idx = (int)(intptr_t)lv_event_get_user_data(e);

    if (LV_EVENT_CLICKED != lv_event_get_code(e)) return;

    if (sg_write_state == 2U) return; /* write in progress, ignore taps */

    if (sg_write_state == 1U) {
        const plist_item_t *pi = playlist_store_get(idx);

        if (pi == NULL) return;
        if (app_nfc_write_card(pi->content_id)) {
            sg_write_state = 2U;
            lv_label_set_text_fmt(sg_status_label, "写卡中：%s，请勿移开卡片…",
                                  pi->label);
            lv_obj_set_style_text_color(sg_status_label,
                                        lv_color_hex(PLIST_YELLOW),
                                        LV_PART_MAIN);
        } else {
            lv_label_set_text(sg_status_label, "无法写卡，请检查配对状态");
            lv_obj_set_style_text_color(sg_status_label,
                                        lv_color_hex(PLIST_RED), LV_PART_MAIN);
        }
        return;
    }

    if (playlist_send_load(idx)) {
        const plist_item_t *pi = playlist_store_get(idx);
        lv_label_set_text_fmt(sg_status_label, "播放：%s",
                              pi != NULL ? pi->label : "");
        lv_obj_set_style_text_color(sg_status_label, lv_color_hex(PLIST_WHITE),
                                    LV_PART_MAIN);
    } else {
        lv_label_set_text(sg_status_label, "未连接播放板，请到 COMM 页连接");
        lv_obj_set_style_text_color(sg_status_label, lv_color_hex(PLIST_YELLOW),
                                    LV_PART_MAIN);
    }
}

static void plist_add_row(const plist_item_t *pi, int idx)
{
    lv_obj_t *row = lv_obj_create(sg_list);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), 48);
    lv_obj_set_style_bg_color(row, lv_color_hex(PLIST_CARD), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x1F1F1F),
                              LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(row, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(row, 14, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(row, plist_row_cb, LV_EVENT_CLICKED,
                        (void *)(intptr_t)idx);

    lv_obj_t *title = lv_label_create(row);
    lv_label_set_text(title, pi->label);
    lv_obj_set_style_text_font(title, &font_cn_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(PLIST_WHITE), LV_PART_MAIN);
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_obj_set_flex_grow(title, 1);

    lv_obj_t *meta = lv_label_create(row);
    if (pi->state == PLIST_ITEM_PROCESSING) {
        lv_label_set_text(meta, "处理中");
        lv_obj_set_style_text_font(meta, &font_cn_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(meta, lv_color_hex(PLIST_YELLOW), LV_PART_MAIN);
    } else if (pi->state == PLIST_ITEM_FAILED) {
        lv_label_set_text(meta, "失败");
        lv_obj_set_style_text_font(meta, &font_cn_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(meta, lv_color_hex(PLIST_RED), LV_PART_MAIN);
    } else {
        char buf[16];
        plist_format_duration(pi->duration_ms, buf, sizeof(buf));
        lv_label_set_text(meta, buf);
        lv_obj_set_style_text_font(meta, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_set_style_text_color(meta, lv_color_hex(PLIST_MUTED), LV_PART_MAIN);
    }
    lv_obj_set_style_pad_left(meta, 10, LV_PART_MAIN);
}

static void plist_show_status(const char *text, uint32_t color)
{
    lv_label_set_text(sg_status_label, text);
    lv_obj_set_style_text_color(sg_status_label, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_add_flag(sg_list, LV_OBJ_FLAG_HIDDEN);
}

static void plist_refresh(void)
{
    switch (playlist_store_status()) {
        case PLIST_STORE_LOADING:
            plist_show_status("加载中…", PLIST_MUTED);
            return;
        case PLIST_STORE_NOT_PAIRED:
            plist_show_status("未配对，请先扫码配对", PLIST_YELLOW);
            return;
        case PLIST_STORE_TOKEN_EXPIRED:
            plist_show_status("授权过期，请重新配对", PLIST_RED);
            return;
        case PLIST_STORE_NET_ERROR:
            plist_show_status("网络错误，请检查 WiFi", PLIST_RED);
            return;
        case PLIST_STORE_IDLE:
        case PLIST_STORE_READY:
        default:
            break;
    }

    int count = playlist_store_count();
    if (count == 0) {
        plist_show_status("暂无声音", PLIST_MUTED);
        return;
    }

    if (sg_write_state == 1U) {
        lv_label_set_text(sg_status_label, "检测到白卡：请选择要写入的声音");
        lv_obj_set_style_text_color(sg_status_label,
                                    lv_color_hex(PLIST_YELLOW), LV_PART_MAIN);
    } else if (sg_write_state != 2U) {
        lv_label_set_text(sg_status_label, "");
    }
    lv_obj_clear_flag(sg_list, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clean(sg_list);
    for (int i = 0; i < count; i++) {
        const plist_item_t *pi = playlist_store_get(i);
        if (pi != NULL) {
            plist_add_row(pi, i);
        }
    }
}

static void plist_ui_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    uint32_t gen = playlist_store_generation();
    if (gen == sg_seen_generation) return;
    sg_seen_generation = gen;

    if (lv_screen_active() != sg_scr) return;

    plist_refresh();
}

static void plist_loaded_cb(lv_event_t *e)
{
    (void)e;
    plist_show_status("加载中…", PLIST_MUTED);
    playlist_store_fetch();
}

static void plist_back_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) return;
    sg_write_state = 0U;
    ui_nav_go_anim(UI_SCR_HOME, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250);
}

void ui_playlist_enter_write_mode(void)
{
    if (sg_write_state == 2U) {
        return; /* a write is already running */
    }
    sg_write_state = 1U;
    if (lv_screen_active() != sg_scr) {
        ui_nav_go_anim(UI_SCR_PLAYLIST, LV_SCR_LOAD_ANIM_MOVE_LEFT, 250);
    } else {
        plist_refresh();
    }
}

void ui_playlist_write_done(bool ok)
{
    sg_write_state = 0U;
    if (lv_screen_active() != sg_scr) {
        return;
    }
    if (ok) {
        lv_label_set_text(sg_status_label, "写入成功，正在弹出");
        lv_obj_set_style_text_color(sg_status_label,
                                    lv_color_hex(PLIST_GREEN), LV_PART_MAIN);
        /* Let the user read the message, then return to the player home. */
        if (sg_back_home_timer == NULL) {
            sg_back_home_timer = lv_timer_create(plist_back_home_timer_cb,
                                                 1200, NULL);
        }
        lv_timer_set_period(sg_back_home_timer, 1200);
        lv_timer_reset(sg_back_home_timer);
        lv_timer_resume(sg_back_home_timer);
    } else {
        lv_label_set_text(sg_status_label, "写入失败，请更换卡片重试");
        lv_obj_set_style_text_color(sg_status_label,
                                    lv_color_hex(PLIST_RED), LV_PART_MAIN);
    }
}

void ui_playlist_init(void)
{
    sg_scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(sg_scr);
    lv_obj_set_style_bg_color(sg_scr, lv_color_hex(PLIST_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(sg_scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(sg_scr, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *back_btn = lv_button_create(sg_scr);
    lv_obj_set_pos(back_btn, 4, 4);
    lv_obj_set_size(back_btn, 48, 28);
    lv_obj_set_style_radius(back_btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x1A1A1A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(back_btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(back_btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(back_btn, plist_back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *back_lbl = lv_label_create(back_btn);
    lv_label_set_text(back_lbl, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(back_lbl, lv_color_hex(PLIST_WHITE), LV_PART_MAIN);
    lv_obj_center(back_lbl);

    lv_obj_t *title = lv_label_create(sg_scr);
    lv_label_set_text(title, "我的声音");
    lv_obj_set_style_text_font(title, &font_cn_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(PLIST_WHITE), LV_PART_MAIN);
    lv_obj_set_pos(title, 64, 10);

    sg_status_label = lv_label_create(sg_scr);
    lv_label_set_text(sg_status_label, "");
    lv_obj_set_style_text_font(sg_status_label, &font_cn_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(sg_status_label, lv_color_hex(PLIST_MUTED), LV_PART_MAIN);
    lv_obj_set_pos(sg_status_label, 16, 44);
    lv_obj_set_width(sg_status_label, 448);

    sg_list = lv_obj_create(sg_scr);
    lv_obj_remove_style_all(sg_list);
    lv_obj_set_pos(sg_list, 8, 68);
    lv_obj_set_size(sg_list, 464, 244);
    lv_obj_set_flex_flow(sg_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(sg_list, 6, LV_PART_MAIN);
    lv_obj_set_scroll_dir(sg_list, LV_DIR_VER);
    lv_obj_add_flag(sg_list, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(sg_scr, plist_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);

    lv_timer_create(plist_ui_timer_cb, 250, NULL);

    ui_nav_register(UI_SCR_PLAYLIST, sg_scr);
    ui_nav_attach_gesture(sg_scr);
}
