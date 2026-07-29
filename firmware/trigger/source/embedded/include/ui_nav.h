#ifndef UI_NAV_H
#define UI_NAV_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UI_SCR_HOME = 0,
    UI_SCR_NFC,
    UI_SCR_WIFI,
    UI_SCR_SERVO,
    UI_SCR_COMM,
    UI_SCR_PAIR,
    UI_SCR_PLAYLIST,
    UI_SCR_COUNT,
} ui_screen_e;

void ui_nav_init(void);
void ui_nav_go(ui_screen_e s);
void ui_nav_go_anim(ui_screen_e s, lv_screen_load_anim_t anim, uint32_t time);
lv_obj_t *ui_nav_screen(ui_screen_e s);

/* Called by sub-module init functions to register their screen and attach
 * gesture navigation. */
void ui_nav_register(ui_screen_e s, lv_obj_t *scr);
void ui_nav_attach_gesture(lv_obj_t *scr);
void ui_nav_add_gesture_rule(ui_screen_e s, lv_dir_t dir, ui_screen_e dest,
                             lv_screen_load_anim_t anim);

/* Attach the press-target recorder (LV_EVENT_PRESSED) used by the gesture
 * filter. Call this on every screen that participates in navigation. */
void ui_nav_attach_press(lv_obj_t *scr);

/* Gesture callback for screens not in the sg_screens[] registry (e.g. NFC
 * write sub-screen). Attach via lv_obj_add_event_cb(scr, cb, LV_EVENT_GESTURE, NULL). */
void ui_nav_go_home_on_bottom_cb(lv_event_t *e);

#ifdef __cplusplus
}
#endif

#endif /* UI_NAV_H */
