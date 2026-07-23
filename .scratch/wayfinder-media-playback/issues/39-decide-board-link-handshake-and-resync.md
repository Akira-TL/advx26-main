# Decide Board Link handshake and reconnect resynchronization

Type: grilling
Status: resolved
Blocked by: 23, 35, 36

## Question

What HELLO/version exchange occurs after GATT connection, and after a reconnect should Trigger query Playback state, resend the current session, restart from the last known position, or stop and wait for a new NFC scan?

## Answer

After characteristic discovery and notification subscription, Trigger Board sends `HELLO`. The exchange establishes:

- protocol major/minor version;
- Trigger and Playback roles;
- a per-boot random `boot_id` for each board;
- maximum reconstructed message size;
- supported command/report capability bits;
- named playback profile support.

An incompatible major version terminates the connection with a visible protocol error. Minor-version differences are accepted only when required capabilities remain present.

BLE disconnection does not stop an active Playback Session. Playback Board continues the current media state and retains the session while its process remains alive.

After reconnection, Trigger sends `GET_STATUS` before issuing normal commands:

- when Playback reports the same `boot_id` and current `session_id`, Trigger adopts the authoritative Playback position/state and refreshes its control panel;
- when Playback has rebooted or no longer holds the session, Trigger resends `LOAD_SESSION`, seeks to the last confirmed position, and restores the last confirmed playing or paused state;
- when Trigger has no current session, it accepts Playback status for display but may send `STOP` to return both boards to a clean idle state.

A successful HELLO begins a new command-cache epoch while session identity remains explicit and independent from BLE connection identity.
