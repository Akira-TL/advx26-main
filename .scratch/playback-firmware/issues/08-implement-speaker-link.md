# Implement A2DP Speaker Link functionality

Type: task
Status: claimed
Blocked by: 01, 07

## Goal

Implement the Playback-side A2DP Source adapter that consumes decoded PCM and exposes stable speaker availability to the media pipeline, without performing physical pairing work in this phase.

## Functional scope

- initialize the classic Bluetooth/A2DP Source capability required by Playback;
- accept the configured fixed-speaker identity through runtime configuration or persisted pairing data;
- expose disconnected, connecting, connected, and streaming conditions through one normalized Speaker Link snapshot;
- feed PCM from the audio path through the A2DP source callback using the negotiated supported format;
- report actual consumed sample counts back to the media clock;
- enter speaker-unavailable behavior without silently advancing media;
- pause PCM consumption on link loss and make automatic resume possible after reconnection;
- reconnect only to the configured fixed target and never select an arbitrary discovered sink.

## Module seam

Expose initialize, connect/reconnect, start/stop PCM, and snapshot operations. Hide Bluetooth profile handles, callback details, negotiated codec state, and controller events.

## Completion criteria

- the media pipeline can distinguish waiting-for-speaker from fatal audio failure;
- PCM consumption is observable in exact samples;
- no speaker picker, substitute-device policy, or pairing UI leaks into Playback engine logic.

## Deferred hardware input

Actual advertised name, Bluetooth address, pairing mode, and physical validation remain in the later debugging-stage speaker ticket.

## Out of scope

Physical pairing, radio coexistence testing, tests, build, flash, and audible-output validation.
