#include "tal_api.h"
#include "lvgl.h"
#include "lv_vendor.h"

#include "ui_nav.h"
#include "ui_home.h"
#include "ui_wifi.h"
#include "ui_servo.h"
#include "ui_comm.h"
#include "ui_pair.h"
#include "ui_playlist.h"
#include "app_nfc.h"

#define UI_NAV_ANIM_MS 250

static lv_obj_t *sg_screens[UI_SCR_COUNT];
static ui_screen_e sg_current = UI_SCR_COUNT;
static lv_obj_t *sg_press_target = NULL;

typedef struct {
    lv_dir_t dir;
    ui_screen_e dest;
    lv_screen_load_anim_t anim;
} gesture_rule_t;

#define MAX_RULES 4

typedef struct {
    gesture_rule_t rules[MAX_RULES];
    uint8_t count;
} gesture_map_t;

static gesture_map_t sg_gesture_maps[UI_SCR_COUNT];

void ui_nav_register(ui_screen_e s, lv_obj_t *scr)
{
    if (s < UI_SCR_COUNT) {
        sg_screens[s] = scr;
    }
}

void ui_nav_add_gesture_rule(ui_screen_e s, lv_dir_t dir, ui_screen_e dest,
                             lv_screen_load_anim_t anim)
{
    gesture_map_t *m = &sg_gesture_maps[s];

    if (m->count < MAX_RULES) {
        m->rules[m->count].dir = dir;
        m->rules[m->count].dest = dest;
        m->rules[m->count].anim = anim;
        m->count++;
    }
}

static bool ui_nav_is_scrollable_ancestor(lv_obj_t *obj, lv_obj_t *scr)
{
    while (obj != NULL && obj != scr) {
        if (lv_obj_has_flag(obj, LV_OBJ_FLAG_SCROLLABLE)) {
            return true;
        }
        obj = lv_obj_get_parent(obj);
    }
    return false;
}

static bool ui_nav_should_navigate(lv_event_t *e)
{
    lv_obj_t *scr = lv_event_get_current_target(e);
    lv_obj_t *press = sg_press_target;

    if (press == NULL || press == scr) {
        return true;
    }
    /* Decorative widgets (panels/labels/dots) opt-in to swipe-through. */
    if (lv_obj_has_flag(press, LV_OBJ_FLAG_USER_1)) {
        return true;
    }
    /* Interactive widgets (buttons, slider, textarea, roller, keyboard) block
     * the swipe so their own gestures (drag slider, scroll list, etc.) win. */
    if (lv_obj_has_flag(press, LV_OBJ_FLAG_CLICKABLE)) {
        return false;
    }
    if (ui_nav_is_scrollable_ancestor(press, scr)) {
        return false;
    }
    return true;
}

static void ui_nav_press_cb(lv_event_t *e)
{
    sg_press_target = lv_event_get_target(e);
}

static void ui_nav_reset_press(lv_event_t *e)
{
    (void)e;
    sg_press_target = NULL;
}

static void ui_nav_gesture_cb(lv_event_t *e)
{
    lv_obj_t *scr = lv_event_get_current_target(e);
    lv_dir_t dir;
    gesture_map_t *m;
    uint8_t i;

    if (!ui_nav_should_navigate(e)) {
        return;
    }

    dir = lv_indev_get_gesture_dir(lv_indev_active());
    sg_press_target = NULL;

    for (i = 0; i < UI_SCR_COUNT; i++) {
        if (sg_screens[i] == scr) {
            break;
        }
    }
    if (i >= UI_SCR_COUNT) {
        return;
    }

    m = &sg_gesture_maps[i];
    for (i = 0; i < m->count; i++) {
        if (m->rules[i].dir == dir) {
            ui_nav_go_anim(m->rules[i].dest, m->rules[i].anim, UI_NAV_ANIM_MS);
            return;
        }
    }
}

void ui_nav_go_home_on_bottom_cb(lv_event_t *e)
{
    if (!ui_nav_should_navigate(e)) {
        return;
    }
    if (lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_BOTTOM) {
        ui_nav_go_anim(UI_SCR_HOME, LV_SCR_LOAD_ANIM_MOVE_BOTTOM,
                       UI_NAV_ANIM_MS);
    }
}

void ui_nav_attach_gesture(lv_obj_t *scr)
{
    lv_obj_add_event_cb(scr, ui_nav_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(scr, ui_nav_reset_press, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(scr, ui_nav_reset_press, LV_EVENT_PRESS_LOST, NULL);
}

void ui_nav_attach_press(lv_obj_t *scr)
{
    lv_obj_add_event_cb(scr, ui_nav_press_cb, LV_EVENT_PRESSED, NULL);
}

void ui_nav_go(ui_screen_e s)
{
    if (s >= UI_SCR_COUNT || sg_screens[s] == NULL || s == sg_current) {
        return;
    }
    sg_current = s;
    lv_screen_load(sg_screens[s]);
}

void ui_nav_go_anim(ui_screen_e s, lv_screen_load_anim_t anim, uint32_t time)
{
    if (s >= UI_SCR_COUNT || sg_screens[s] == NULL || s == sg_current) {
        return;
    }
    sg_current = s;
    lv_screen_load_anim(sg_screens[s], anim, time, 0, false);
}

lv_obj_t *ui_nav_screen(ui_screen_e s)
{
    return (s < UI_SCR_COUNT) ? sg_screens[s] : NULL;
}

void ui_nav_init(void)
{
    /*
     * PWM0 defaults to GPIO18, which is also the RGB LCD VSYNC pin.
     * Remap PWM0 to the confirmed servo pin before the display starts so
     * lv_vendor_init() can make GPIO18 the final VSYNC owner.
     */
    ui_servo_hw_init();

    lv_vendor_init(DISPLAY_NAME);
    lv_vendor_disp_lock();

    ui_home_init();
    app_nfc_init();
    ui_wifi_init();
    ui_servo_init();
    ui_comm_init();
    ui_pair_init();
    ui_playlist_init();

    ui_nav_add_gesture_rule(UI_SCR_HOME, LV_DIR_BOTTOM, UI_SCR_COMM,
                            LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    ui_nav_add_gesture_rule(UI_SCR_HOME, LV_DIR_RIGHT, UI_SCR_WIFI,
                            LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    ui_nav_add_gesture_rule(UI_SCR_HOME, LV_DIR_LEFT, UI_SCR_PAIR,
                            LV_SCR_LOAD_ANIM_MOVE_LEFT);
    ui_nav_add_gesture_rule(UI_SCR_COMM, LV_DIR_TOP, UI_SCR_HOME,
                            LV_SCR_LOAD_ANIM_MOVE_TOP);
    ui_nav_add_gesture_rule(UI_SCR_WIFI, LV_DIR_LEFT, UI_SCR_HOME,
                            LV_SCR_LOAD_ANIM_MOVE_LEFT);
    ui_nav_add_gesture_rule(UI_SCR_SERVO, LV_DIR_RIGHT, UI_SCR_HOME,
                            LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    ui_nav_add_gesture_rule(UI_SCR_HOME, LV_DIR_TOP, UI_SCR_PLAYLIST,
                            LV_SCR_LOAD_ANIM_MOVE_TOP);
    ui_nav_add_gesture_rule(UI_SCR_PLAYLIST, LV_DIR_BOTTOM, UI_SCR_HOME,
                            LV_SCR_LOAD_ANIM_MOVE_BOTTOM);
    ui_nav_add_gesture_rule(UI_SCR_PAIR, LV_DIR_RIGHT, UI_SCR_HOME,
                            LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    ui_nav_add_gesture_rule(UI_SCR_PLAYLIST, LV_DIR_RIGHT, UI_SCR_HOME,
                            LV_SCR_LOAD_ANIM_MOVE_RIGHT);

    for (uint8_t i = 0; i < UI_SCR_COUNT; i++) {
        if (sg_screens[i] != NULL) {
            ui_nav_attach_press(sg_screens[i]);
        }
    }

    lv_screen_load(sg_screens[UI_SCR_HOME]);
    sg_current = UI_SCR_HOME;

    lv_vendor_disp_unlock();
    lv_vendor_start(THREAD_PRIO_1, 1024 * 8);
}
