# Decide automatic start and end-of-media behavior

Type: grilling
Status: resolved
Blocked by: 07, 11, 12

## Question

After the Trigger Board resolves NFC and loads a session, should playback start automatically or wait for user confirmation, and what should both screens show when the media reaches the end?

## Answer

A valid NFC scan starts playback automatically:

1. Trigger Board resolves the Compact Content URL.
2. Trigger Board renders the Trigger Control Panel and sends `LOAD_SESSION`.
3. Playback Board downloads and prepares the device assets.
4. After a successful ready report, Trigger Board sends `PLAY` without requiring another tap.

When playback reaches the end:

- Playback Board holds the final video frame on screen;
- Playback Board reports `COMPLETED`;
- Trigger Board shows the completed state and exposes a replay action;
- replay seeks to the beginning and starts the same session again.

A newer valid NFC scan still interrupts and replaces the current session immediately.
