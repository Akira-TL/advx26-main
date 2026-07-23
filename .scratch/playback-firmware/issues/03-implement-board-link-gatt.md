# Implement Board Link GATT peripheral

Type: task
Status: open
Blocked by: 02

## Goal

Expose Playback as the fixed Board Link BLE peripheral and bridge GATT traffic to the codec without embedding playback policy in Bluetooth callbacks.

## Functional scope

- advertise the fixed Board Link Service UUID and Playback device role;
- register the Command characteristic as write-with-response;
- register the Report characteristic as Notify;
- accept ATT fragments, pass them to the Board Link codec, and enqueue complete commands for Playback processing outside the Bluetooth callback;
- fragment encoded reports according to negotiated ATT payload capacity minus the 16-byte header;
- enable Report notifications only after the client subscribes;
- generate one boot ID per firmware boot;
- support HELLO/HELLO_ACK capability negotiation and reject incompatible major versions;
- expose connection/subscription state to the Playback engine without treating disconnect as STOP;
- continuously remain discoverable/reconnectable according to the existing fixed-service discovery decision.

## Module seam

Expose command-delivery and report-send callbacks. Hide Tuya/Beken GATT handles, characteristic indexes, MTU details, and callback threading from the Playback engine.

## Completion criteria

- a complete decoded command reaches one serialized command queue;
- reports are delivered only through the fixed Report characteristic;
- BLE disconnect preserves the active Playback session in memory;
- no Trigger, NFC, or BLE Central behavior is introduced.

## Out of scope

Media execution, pairing UI, bonding UI, tests, build, flash, and radio coexistence validation.
