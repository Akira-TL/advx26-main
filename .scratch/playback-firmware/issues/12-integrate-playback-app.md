# Integrate Playback application bootstrap

Type: task
Status: open
Blocked by: 11

## Goal

Replace the fixed JPEG demo startup path with the complete Board Link-driven MP4/H.264 Playback application while keeping `main.c` as a small bootstrap caller.

## Functional scope

- initialize logging, display, Wi-Fi, Bluetooth, Board Link, Playback engine, MP4/H.264 media workers, MP3 workers, and Speaker Link in a deterministic order;
- keep fixed demo Wi-Fi and Cloud Media Service configuration in the existing Git-ignored runtime configuration seam;
- replace automatic `DEMO_VIDEO_URL` playback with Board Link-driven `LOAD_SESSION` behavior;
- preserve an idle Playback screen before the first session;
- map `LOADING`, `WAITING_SPEAKER`, `BUFFERING`, `ERROR`, and `COMPLETED` to minimal output-only screen behavior while keeping Trigger as the interaction owner;
- keep the final decoded video frame visible after natural completion;
- release MP4 metadata, H.264/MP3 decoder state, buffers, and active reads on STOP and replacement;
- remove the temporary JSON JPEG index, per-frame URLs, JPEG decoder path, and elapsed-time video scheduler after reusable HTTP/display code has migrated;
- centralize task creation, configuration ownership, and shutdown ordering so `main.c` exposes no media-container or codec details.

## Module seam

`main.c` invokes one Playback application bootstrap interface. Application composition owns concrete adapters; business state remains inside `playback_engine` and media behavior remains inside `media_pipeline`.

## Completion criteria

- firmware no longer starts media solely because Wi-Fi connected;
- a valid `t5ai-h264-mp3-v1` Board Link session is the only normal entry into media preparation;
- startup code exposes no temporary JPEG profile, per-frame URL, or demo-URL assumptions;
- the Playback functional path is ready for later tests, build, flash, speaker pairing, and hardware acceptance phases.

## Out of scope

Tests, test-stub changes, build, flash, serial acceptance, physical speaker pairing, and Trigger/backend/App work.
