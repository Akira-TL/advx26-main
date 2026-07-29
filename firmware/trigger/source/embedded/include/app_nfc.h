#ifndef APP_NFC_H
#define APP_NFC_H

#include <stdbool.h>
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

OPERATE_RET app_nfc_init(void);

/*
 * Write "{server}/c/{content_id}" as an NDEF URI to the card on the reader.
 * Retries automatically (1 s apart, 3 attempts total); on success or final
 * failure it notifies ui_playlist_write_done() and runs the servo eject.
 * Returns false when a write is already running or server/id is missing.
 * LVGL timer context only.
 */
bool app_nfc_write_card(const char *content_id);

#ifdef __cplusplus
}
#endif

#endif /* APP_NFC_H */
