#ifndef UI_HOME_H
#define UI_HOME_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_home_init(void);

/*
 * Switch the player to playlist item idx and start playing (sends a TCP
 * load). loop=true keeps replaying the same item at the end of its
 * duration until the user manually changes track. Safe to call from LVGL
 * timer context only.
 */
void ui_home_play_index(int idx, bool loop);

/* Reveal the home-screen eject button (hidden until an NFC card is tapped;
 * hides itself again after being pressed). */
void ui_home_show_eject(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_HOME_H */
