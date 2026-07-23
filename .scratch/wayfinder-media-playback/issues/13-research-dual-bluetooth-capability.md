# Research simultaneous Board Link and Speaker Link capability

Type: research
Status: resolved
Blocked by:

## Question

Can TuyaOpen v1.9.0 on T5AI simultaneously maintain a BLE GATT Board Link and a Bluetooth audio Speaker Link? Identify the supported Bluetooth roles, examples, coexistence constraints, and the smallest viable architecture from primary SDK sources.

## Answer

The SDK exposes classic Bluetooth and BLE in the same T5AI configuration, a complete A2DP Source PCM input path, and BLE GATT write/notify support. Use the Playback Board as BLE peripheral/GATT server plus A2DP Source, and the Trigger Board as BLE central/GATT client.

No bundled example proves both links under sustained simultaneous load, so coexistence remains gated by an early hardware smoke test rather than assumed complete.

Research asset: [Dual Bluetooth capability research](../research/13-dual-bluetooth-capability.md).
