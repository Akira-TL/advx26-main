#ifndef UI_PAIR_H
#define UI_PAIR_H

#ifdef __cplusplus
extern "C" {
#endif

/* QR-code pairing screen: shows a QR code that the SoundPola app scans, then
 * receives the user's cloud access token over LAN HTTP and persists it. */
void ui_pair_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_PAIR_H */
