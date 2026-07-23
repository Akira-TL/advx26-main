# Prototype simultaneous Board Link and Speaker Link

Type: prototype
Status: open
Blocked by: 13, 41, 43, 44

## Question

Can the Playback Board sustain a BLE peripheral/GATT session and an A2DP Source connection to the fixed Bluetooth Speaker at the same time without disconnects, controller errors, PCM underruns, or audible artifacts?

The prototype must keep both links active for at least 60 seconds while exchanging representative commands and progress notifications and continuously feeding PCM audio.
