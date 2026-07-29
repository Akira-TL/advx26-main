#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "tal_api.h"
#include "tkl_pwm.h"
#include "lvgl.h"

#include "ui_servo.h"
#include "ui_nav.h"

#define SERVO_BG           0x111827
#define SERVO_CARD         0x1F2A37
#define SERVO_WHITE        0xF7FBFF
#define SERVO_MUTED        0xA9C1D7
#define SERVO_CYAN         0x48D8FF
#define SERVO_GREEN        0x48E0A4
#define SERVO_RED          0xFF6B7A
#define SERVO_ACCENT       0x48D8FF

/* Servo signal is wired to P11 pin 9: P06 / SFC_IO2. T5AI can output PWM0 on
 * GPIO6. After PWM0 init we restore GPIO18 to LCD_VSYNC and route PWM0 to
 * GPIO6, otherwise PWM0's default GPIO18 mapping can steal the LCD VSYNC pin. */
#define SERVO_PWM_CH         TUYA_PWM_NUM_0
#define SERVO_SIGNAL_GPIO    6
#define SERVO_LCD_VSYNC_GPIO 18
#define SERVO_DEV_PWM0       0x01
#define SERVO_DEV_LCD_VSYNC  0xAB
#define SERVO_UPDATE_MS      20U
#define SERVO_DEG_PER_SEC    90U
#define SERVO_CMD_DEG        180
#define SERVO_NFC_HOLD_MS    1500U
/* Continuous-rotation 360-degree servo: duty is speed, not position.
 * 750 = 1.5 ms stop, 250/1250 = full speed CCW/CW. */
#define SERVO_CCW_DUTY       250U
#define SERVO_NEUTRAL_DUTY   750U
#define SERVO_CW_DUTY        1250U

extern int gpio_dev_map(int gpio_id, int dev);
extern int gpio_dev_unmap(int gpio_id);

static const int16_t sg_waypoints[] = {
    0, 90, 180, 270, 356, 270, 180, 90, 0
};

static lv_obj_t *sg_scr;
static lv_obj_t *sg_status_label;
static lv_obj_t *sg_angle_label;
static lv_timer_t *sg_motion_timer;
static lv_timer_t *sg_pulse_timer;

static int sg_command_angle;
static uint8_t sg_motion_segment;
static uint32_t sg_segment_elapsed_ms;

static bool servo_route_pwm_to_p11_pin9(void)
{
    if (gpio_dev_unmap(SERVO_LCD_VSYNC_GPIO) != 0 ||
        gpio_dev_map(SERVO_LCD_VSYNC_GPIO, SERVO_DEV_LCD_VSYNC) != 0) {
        PR_ERR("restore LCD VSYNC GPIO18 failed");
        return false;
    }

    if (gpio_dev_unmap(SERVO_SIGNAL_GPIO) != 0 ||
        gpio_dev_map(SERVO_SIGNAL_GPIO, SERVO_DEV_PWM0) != 0) {
        PR_ERR("map servo PWM0 to GPIO6/P11-9 failed");
        return false;
    }

    return true;
}

void ui_servo_hw_init(void)
{
    static bool inited = false;
    TUYA_PWM_BASE_CFG_T cfg;

    if (inited) {
        return;
    }

    memset(&cfg, 0, sizeof(cfg));
    cfg.polarity  = TUYA_PWM_POSITIVE;
    cfg.count_mode = TUYA_PWM_CNT_UP;
    cfg.frequency = 50;        /* 50 Hz, 20 ms period. */
    cfg.duty      = SERVO_NEUTRAL_DUTY;
    cfg.cycle     = 10000;

    if (tkl_pwm_init(SERVO_PWM_CH, &cfg) != OPRT_OK) {
        PR_ERR("tkl_pwm_init failed");
        return;
    }
    if (!servo_route_pwm_to_p11_pin9()) {
        return;
    }
    if (tkl_pwm_start(SERVO_PWM_CH) != OPRT_OK) {
        PR_ERR("tkl_pwm_start failed");
        return;
    }
    inited = true;
    PR_NOTICE("servo PWM ready on GPIO6 / P11 pin 9 (PWM0)");
}

static void servo_set_status(uint32_t color, const char *text)
{
    if (sg_status_label == NULL) {
        return;
    }
    lv_obj_set_style_text_color(sg_status_label, lv_color_hex(color),
                                LV_PART_MAIN);
    lv_label_set_text(sg_status_label, text);
}

static void servo_set_angle_label(int angle, const char *state)
{
    char buf[48];

    if (sg_angle_label == NULL) {
        return;
    }
    snprintf(buf, sizeof(buf), "Command: %+d\u00B0  %s", angle, state);
    lv_obj_set_style_text_color(sg_angle_label, lv_color_hex(SERVO_GREEN),
                                LV_PART_MAIN);
    lv_label_set_text(sg_angle_label, buf);
}

static bool servo_set_duty(uint32_t duty)
{
    OPERATE_RET rt = tkl_pwm_duty_set(SERVO_PWM_CH, duty);

    PR_NOTICE("servo duty=%u rt=%d", (unsigned)duty, rt);
    if (rt != OPRT_OK) {
        servo_set_status(SERVO_RED, "PWM duty set failed");
        return false;
    }
    return true;
}

static bool servo_set_command(int command_angle, const char *state)
{
    uint32_t duty = SERVO_NEUTRAL_DUTY;

    if (command_angle > 0) {
        duty = SERVO_CW_DUTY;
    } else if (command_angle < 0) {
        duty = SERVO_CCW_DUTY;
    }

    if (!servo_set_duty(duty)) {
        return false;
    }

    sg_command_angle = command_angle;
    servo_set_angle_label(sg_command_angle, state);
    return true;
}

static bool servo_start_segment(void)
{
    int from = sg_waypoints[sg_motion_segment];
    int to = sg_waypoints[sg_motion_segment + 1U];
    int command_angle = (to > from) ? -SERVO_CMD_DEG : SERVO_CMD_DEG;
    const char *state = (command_angle > 0) ? "CW" : "CCW";

    sg_segment_elapsed_ms = 0;
    PR_NOTICE("servo virtual %d -> %d, command %+d deg", from, to,
              command_angle);
    return servo_set_command(command_angle, state);
}

static void servo_pulse_timer_cb(lv_timer_t *timer)
{
    servo_set_command(0, "Stopped");
    servo_set_status(SERVO_GREEN, "NFC pulse complete");
    lv_timer_pause(timer);
}

void ui_servo_pulse_180(void)
{
    ui_servo_hw_init();

    if (sg_motion_timer != NULL) {
        lv_timer_pause(sg_motion_timer);
    }
    if (!servo_set_command(SERVO_CMD_DEG, "NFC")) {
        return;
    }
    servo_set_status(SERVO_GREEN, "NFC pulse +180");

    if (sg_pulse_timer == NULL) {
        sg_pulse_timer = lv_timer_create(servo_pulse_timer_cb,
                                         SERVO_NFC_HOLD_MS, NULL);
    }
    lv_timer_set_period(sg_pulse_timer, SERVO_NFC_HOLD_MS);
    lv_timer_reset(sg_pulse_timer);
    lv_timer_resume(sg_pulse_timer);
}

static lv_timer_t *sg_eject_timer;
/* 0 idle, 1 spinning out 360 */
static uint8_t sg_eject_stage;

/* Continuous servo: one full revolution at full speed takes about this long. */
#define SERVO_EJECT_SPIN_MS  1500U

static void servo_eject_timer_cb(lv_timer_t *timer)
{
    servo_set_command(0, "Stopped");
    servo_set_status(SERVO_GREEN, "Eject complete");
    sg_eject_stage = 0U;
    lv_timer_pause(timer);
}

void ui_servo_eject(void)
{
    ui_servo_hw_init();

    if (sg_eject_stage != 0U) {
        return;
    }
    if (sg_motion_timer != NULL) {
        lv_timer_pause(sg_motion_timer);
    }
    if (sg_pulse_timer != NULL) {
        lv_timer_pause(sg_pulse_timer);
    }
    if (!servo_set_command(SERVO_CMD_DEG, "Eject")) {
        return;
    }
    servo_set_status(SERVO_GREEN, "Eject 360 out");
    sg_eject_stage = 1U;

    if (sg_eject_timer == NULL) {
        sg_eject_timer = lv_timer_create(servo_eject_timer_cb,
                                         SERVO_EJECT_SPIN_MS, NULL);
    }
    lv_timer_set_period(sg_eject_timer, SERVO_EJECT_SPIN_MS);
    lv_timer_reset(sg_eject_timer);
    lv_timer_resume(sg_eject_timer);
}

static void servo_motion_timer_cb(lv_timer_t *timer)
{
    int from = sg_waypoints[sg_motion_segment];
    int to = sg_waypoints[sg_motion_segment + 1U];
    int delta = to - from;
    uint32_t distance = (uint32_t)((delta < 0) ? -delta : delta);
    uint32_t duration_ms = (distance * 1000U + SERVO_DEG_PER_SEC - 1U) /
                           SERVO_DEG_PER_SEC;

    sg_segment_elapsed_ms += SERVO_UPDATE_MS;
    if (sg_segment_elapsed_ms >= duration_ms) {
        sg_motion_segment++;

        if (sg_motion_segment >=
            (sizeof(sg_waypoints) / sizeof(sg_waypoints[0])) - 1U) {
            servo_set_command(0, "Stopped");
            servo_set_status(SERVO_GREEN, "Sequence complete");
            lv_timer_pause(timer);
            return;
        }

        if (!servo_start_segment()) {
            servo_set_command(0, "Stopped");
            lv_timer_pause(timer);
        }
        return;
    }
}

static lv_obj_t *servo_create_label(lv_obj_t *parent, const char *text,
                                    const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    /* Decorative label is swipe-through: USER_1 wins over CLICKABLE in the
     * gesture filter. Empty default PRESSED style avoids a flash while swiping. */
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(label, LV_OBJ_FLAG_USER_1);
    return label;
}

static void servo_back_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) {
        return;
    }
    ui_nav_go_anim(UI_SCR_HOME, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250);
}

static void servo_confirm_cb(lv_event_t *e)
{
    if (LV_EVENT_CLICKED != lv_event_get_code(e)) {
        return;
    }

    sg_motion_segment = 0;
    sg_command_angle = 0;
    servo_set_angle_label(0, "Starting");
    servo_set_status(SERVO_MUTED, "Sequence running...");
    if (!servo_start_segment()) {
        return;
    }
    lv_timer_reset(sg_motion_timer);
    lv_timer_resume(sg_motion_timer);
}

void ui_servo_init(void)
{
    lv_obj_t *btn;
    lv_obj_t *label;
    lv_obj_t *title;

    ui_servo_hw_init();

    sg_scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(sg_scr);
    lv_obj_set_style_bg_color(sg_scr, lv_color_hex(SERVO_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sg_scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_clear_flag(sg_scr, LV_OBJ_FLAG_SCROLLABLE);
    /* Screen must be clickable so background presses produce a recorded
     * press target and the gesture handler can fire. */
    lv_obj_add_flag(sg_scr, LV_OBJ_FLAG_CLICKABLE);

    /* Top bar: back button (left) + centered title. */
    btn = lv_button_create(sg_scr);
    lv_obj_set_pos(btn, 4, 4);
    lv_obj_set_size(btn, 40, 28);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(SERVO_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, servo_back_cb, LV_EVENT_CLICKED, NULL);
    label = servo_create_label(btn, LV_SYMBOL_LEFT, &lv_font_montserrat_14,
                               SERVO_WHITE);
    lv_obj_center(label);

    title = servo_create_label(sg_scr, "Servo Control",
                               &lv_font_montserrat_20, SERVO_WHITE);
    lv_obj_set_pos(title, 48, 8);

    /* Motion sequence description. */
    {
        lv_obj_t *hint = servo_create_label(sg_scr,
            "Rise=-180\u00B0, fall=+180\u00B0, end=0\u00B0",
            &lv_font_montserrat_14, SERVO_MUTED);
        lv_obj_set_pos(hint, 0, 74);
        lv_obj_set_width(hint, 480);
        lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }

    /* Run/restart button. */
    btn = lv_button_create(sg_scr);
    lv_obj_set_pos(btn, 140, 174);
    lv_obj_set_size(btn, 200, 48);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(SERVO_ACCENT), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, servo_confirm_cb, LV_EVENT_CLICKED, NULL);
    label = servo_create_label(btn, "RUN SEQUENCE", &lv_font_montserrat_20,
                               SERVO_BG);
    lv_obj_center(label);

    /* Current command angle / pulse width (the "happy" label, green). */
    sg_angle_label = servo_create_label(sg_scr, "", &lv_font_montserrat_14,
                                        SERVO_GREEN);
    lv_obj_set_pos(sg_angle_label, 0, 122);
    lv_obj_set_width(sg_angle_label, 480);
    lv_obj_set_style_text_align(sg_angle_label, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN);

    /* Status (warnings / errors). */
    sg_status_label = servo_create_label(sg_scr, "", &lv_font_montserrat_14,
                                         SERVO_MUTED);
    lv_obj_set_pos(sg_status_label, 0, 242);
    lv_obj_set_width(sg_status_label, 480);
    lv_obj_set_style_text_align(sg_status_label, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN);

    servo_set_command(0, "Stopped");
    servo_set_status(SERVO_MUTED, "Ready - three-state motion");
    sg_motion_timer = lv_timer_create(servo_motion_timer_cb,
                                      SERVO_UPDATE_MS, NULL);
    lv_timer_pause(sg_motion_timer);

    ui_nav_register(UI_SCR_SERVO, sg_scr);
    ui_nav_attach_gesture(sg_scr);
}
