#ifndef TEST_STUB_LVGL_H
#define TEST_STUB_LVGL_H

#include <stdint.h>

typedef struct lv_obj_t lv_obj_t;
typedef struct lv_event_t lv_event_t;
typedef void (*lv_event_cb_t)(lv_event_t *event);
typedef uint32_t lv_color_t;
typedef int32_t lv_coord_t;
typedef int lv_text_align_t;
typedef int lv_align_t;

#define LV_OPA_TRANSP 0
#define LV_OPA_COVER 255
#define LV_OBJ_FLAG_SCROLLABLE (1U << 0)
#define LV_EVENT_CLICKED 1

#define LV_TEXT_ALIGN_LEFT 0
#define LV_TEXT_ALIGN_CENTER 1

#define LV_ALIGN_CENTER 0
#define LV_ALIGN_TOP_MID 1
#define LV_ALIGN_LEFT_MID 2
#define LV_ALIGN_RIGHT_MID 3
#define LV_ALIGN_TOP_LEFT 4
#define LV_ALIGN_BOTTOM_LEFT 5
#define LV_ALIGN_BOTTOM_RIGHT 6
#define LV_ALIGN_BOTTOM_MID 7

static inline lv_color_t lv_color_hex(uint32_t value)
{
    return value;
}

static inline lv_coord_t lv_pct(int32_t value)
{
    return value;
}

lv_obj_t *lv_obj_create(lv_obj_t *parent);
lv_obj_t *lv_btn_create(lv_obj_t *parent);
lv_obj_t *lv_label_create(lv_obj_t *parent);
void lv_label_set_text(lv_obj_t *object, const char *text);
void lv_obj_clear_flag(lv_obj_t *object, uint32_t flag);
void lv_obj_add_event_cb(
    lv_obj_t *object,
    lv_event_cb_t callback,
    int32_t filter,
    void *user_data
);
lv_obj_t *lv_event_get_target(lv_event_t *event);
void lv_obj_set_size(lv_obj_t *object, lv_coord_t width, lv_coord_t height);
void lv_obj_set_style_bg_color(lv_obj_t *object, lv_color_t color, int selector);
void lv_obj_set_style_bg_opa(lv_obj_t *object, int opacity, int selector);
void lv_obj_set_style_border_color(lv_obj_t *object, lv_color_t color, int selector);
void lv_obj_set_style_border_width(lv_obj_t *object, int32_t width, int selector);
void lv_obj_set_style_pad_all(lv_obj_t *object, int32_t value, int selector);
void lv_obj_set_style_radius(lv_obj_t *object, int32_t radius, int selector);
void lv_obj_set_style_text_color(lv_obj_t *object, lv_color_t color, int selector);
void lv_obj_set_style_text_align(lv_obj_t *object, lv_text_align_t alignment, int selector);
void lv_obj_set_style_text_letter_space(lv_obj_t *object, int32_t value, int selector);
void lv_obj_set_style_text_line_space(lv_obj_t *object, int32_t value, int selector);
void lv_obj_align(lv_obj_t *object, lv_align_t alignment, lv_coord_t x, lv_coord_t y);
void lv_obj_center(lv_obj_t *object);
void lv_disp_load_scr(lv_obj_t *screen);

#endif /* TEST_STUB_LVGL_H */
