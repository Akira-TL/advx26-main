# Define Playback domain and state model

Type: task
Status: resolved
Blocked by:

## Goal

Create the Playback-side domain types and transition rules required by every later module, without adding transport, HTTP, decoder, or hardware behavior.

## Functional scope

Define:

- `playback_session_t` with `session_id`, immutable `content_id`, duration, profile, asset descriptors, and autoplay/end behavior;
- `playback_command_t` for `HELLO`, `GET_STATUS`, `LOAD_SESSION`, `PLAY`, `PAUSE`, `SEEK_MS`, and `STOP`;
- `playback_report_t` for handshake, ACK/NACK, state, progress, completion, and error reports;
- the fixed Playback states and error enums from Wayfinder issues 51 and 52;
- `playback_snapshot_t` containing authoritative session, state, position, duration, play/pause intent, boot ID, and retryable error metadata;
- a transition function that rejects illegal transitions and centralizes state invariants.

## Module seam

Expose domain values and pure transition helpers only. Do not expose HTTP handles, BLE structures, decoder contexts, LVGL objects, or A2DP types.

## Completion criteria

- all later modules can depend on one canonical Playback vocabulary;
- state changes are represented through one transition path rather than direct enum assignment across callers;
- STOP, replacement, completion, waiting-speaker, buffering, and seek intent rules are expressible without free-form flags.

## Out of scope

Board Link serialization, media loading, audio/video execution, tests, build, flash, and hardware debugging.

## Answer

Implemented the canonical Playback domain in `firmware/playback` with enum-driven commands, reports, states, errors, session descriptors, authoritative snapshots, and centralized transition validation. The implementation is recorded in playback commit `ea108e1` and synchronized through the later playback foundation commits.
