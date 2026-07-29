#ifndef UI_SERVO_H
#define UI_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

void ui_servo_init(void);
void ui_servo_hw_init(void);
void ui_servo_pulse_180(void);
void ui_servo_eject(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_SERVO_H */
