# Trigger Firmware — Reserved

This location is reserved for the SoundPola Trigger Board firmware. No Trigger implementation is maintained here yet.

Planned responsibilities:

- receive a Compact Content URL from the PN532-facing application seam;
- resolve content metadata through the Cloud Media Service;
- act as BLE Central for the Board Link;
- render the Trigger Control Panel;
- issue playback commands and display Playback reports.

The PN532 wiring, pin assignment, electrical interface, and low-level transport driver remain outside this repository boundary.

When development starts, replace this reserved directory with a dedicated Trigger Git repository/submodule. Do not add Trigger code to `firmware/playback/`.
