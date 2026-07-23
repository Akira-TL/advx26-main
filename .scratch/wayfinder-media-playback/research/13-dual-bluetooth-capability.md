# Dual Bluetooth capability research

## Conclusion

TuyaOpen v1.9.0 and the T5AI/Beken platform expose the pieces required for a BLE Board Link and an A2DP Source Speaker Link in one firmware image, but the repository does not contain a ready-made end-to-end example proving both links under simultaneous sustained load. Treat the architecture as supported enough to prototype, not yet hardware-verified.

## Primary-source evidence

- The T5AI headset project enables classic Bluetooth and BLE together (`CONFIG_BT=y`, `CONFIG_BT_AT_ENABLE=y`, `CONFIG_BLE_AT_ENABLE=y`) while also enabling media, SBC, and an A2DP profile: `/home/akira/SDKs/TuyaOpen-v1.9.0/platform/T5AI/t5_os/projects/bluetooth/headset/ap/config/bk7258_ap/config:15-27`.
- The platform public A2DP API provides Source initialization, PCM data callbacks, PCM format selection, resampling, connection, and media control: `/home/akira/SDKs/TuyaOpen-v1.9.0/platform/T5AI/t5_os/ap/include/components/bluetooth/bk_dm_a2dp.h:24-148`.
- A2DP Source accepts PCM at 8/16/32/44.1/48 kHz, 16-bit, one or two channels: `bk_dm_a2dp.h:62-108`.
- Tuya's BLE abstraction creates write and notify characteristics with 244-byte values and exposes server notifications: `/home/akira/SDKs/TuyaOpen-v1.9.0/src/tal_bluetooth/src/tal_bluetooth.c:500-533` and `:867-909`.

## Recommended roles

- **Playback Board**: BLE peripheral/GATT server plus classic Bluetooth A2DP Source.
- **Trigger Board**: BLE central/GATT client.

This keeps discovery and active initiation for the Speaker Link on the Playback Board while the Board Link remains a low-bandwidth peripheral connection.

## Protocol implications

- Use one custom GATT service.
- Use a write characteristic for Trigger Board commands and session setup.
- Use notify for periodic progress and non-critical state reports.
- Use write-with-response or an explicit application acknowledgement for commands that change session state.
- Keep messages well below the negotiated MTU and fragment larger session descriptors at the application layer.

## Required prototype gate

Before implementation tickets assume coexistence, run a hardware smoke test that simultaneously:

1. keeps the Playback Board connected as a BLE peripheral;
2. connects it as A2DP Source to the fixed speaker;
3. continuously feeds PCM audio for at least 60 seconds;
4. exchanges BLE commands and progress notifications at the intended rate;
5. records disconnects, underruns, controller errors, and audio artifacts.
