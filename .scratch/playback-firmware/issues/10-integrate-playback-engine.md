# Integrate Playback engine commands, state, and reports

Type: task
Status: resolved
Blocked by: 01, 03, 04, 09

## Goal

Implement `playback_engine` as the single policy owner that translates Board Link commands into media-pipeline actions and publishes authoritative reports.

## Functional scope

- initialize with runtime configuration and one injected report sink;
- accept one command at a time from the Board Link queue;
- enforce sequence ID monotonicity, one in-flight command, recent-result caching, and duplicate replay without re-execution;
- reject reuse of the same sequence ID with different command content;
- implement `LOAD_SESSION`, `PLAY`, `PAUSE`, `SEEK_MS`, `STOP`, `GET_STATUS`, and handshake-facing behavior;
- replace the active session immediately when a newer valid `LOAD_SESSION` is accepted;
- map media-pipeline snapshots into stable Playback states;
- send immediate ACK/NACK, immediate state changes, 500 ms progress reports while playing, completion, and fatal errors;
- keep Trigger-visible state authoritative even when BLE disconnects;
- support replay as seek-to-zero followed by play without changing `session_id`.

## Module seam

Callers know only engine initialization, command submission, and snapshot retrieval. Session replacement, idempotency, report correlation, and media-policy decisions stay inside the module.

## Completion criteria

- no Bluetooth callback directly mutates playback state;
- no media module serializes Board Link JSON;
- every accepted command produces exactly one correlated final ACK/NACK result;
- STOP reliably releases active media and returns the engine to IDLE.

## Implementation result

- Added a dedicated engine worker so Board Link callbacks only copy and enqueue commands.
- Enforced monotonic sequence IDs, semantic command fingerprints, an eight-result cache, duplicate replay, and conflict NACKs.
- `LOAD_SESSION` now ACKs immediately, publishes `LOADING`, downloads and validates the MP3 index, then prepares the scheduler.
- Implemented `PLAY`, `PAUSE`, `SEEK_MS`, `STOP`, `GET_STATUS`, 500 ms progress, completion, and fatal-error reports.
- Authoritative snapshots remain available independently of BLE connectivity, and STOP clears failed or active sessions back to IDLE.

## Out of scope

Reconnect restoration details, tests, build, flash, and Trigger implementation.
