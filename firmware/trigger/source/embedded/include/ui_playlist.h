#ifndef UI_PLAYLIST_H
#define UI_PLAYLIST_H

#include <stdbool.h>

void ui_playlist_init(void);

/*
 * A blank NFC card was detected: switch to the playlist screen in write
 * mode, where tapping a row writes that song's URL to the card instead of
 * playing it. LVGL timer context only.
 */
void ui_playlist_enter_write_mode(void);

/* Result callback for the write started from write mode. */
void ui_playlist_write_done(bool ok);

#endif /* UI_PLAYLIST_H */
