# Decide Playback state and error vocabulary

Type: grilling
Status: resolved
Blocked by: 11, 16, 17, 40, 46

## Question

Which finite Playback states and stable error categories must Trigger understand, and which controls remain available during loading, buffering, seeking, speaker wait, completion, and failure?

## Answer

The first protocol freezes a bounded Playback state machine. Trigger must make behavior decisions from stable enums, never from human-readable status text.

Playback states are:

- `IDLE`: no active prepared session;
- `LOADING`: validating descriptors, opening MP4 metadata, or loading the audio index;
- `WAITING_SPEAKER`: media is prepared but the configured A2DP Sink is unavailable;
- `BUFFERING`: filling compressed H.264/MP3 and decoded video/PCM buffers before media time advances;
- `PLAYING`: the PCM-derived media clock advances;
- `PAUSED`: session and position are retained while the media clock is frozen;
- `SEEKING`: MP4 sync-sample selection, decoder reset, forward decode, and audio repositioning are in progress;
- `COMPLETED`: final decoded video frame is held and replay is available;
- `ERROR`: the active session cannot continue without a new command or external recovery.

Stable error codes are:

- `NETWORK_TIMEOUT`;
- `NETWORK_RANGE_INVALID`;
- `CONTENT_INVALID`;
- `MP4_DEMUX_FAILED`;
- `INDEX_INVALID`;
- `H264_DECODE_FAILED`;
- `MP3_DECODE_FAILED`;
- `PROTOCOL_INCOMPATIBLE`;
- `INTERNAL_ERROR`.

Every `ERROR` report includes `code`, `retryable`, the current `position_ms`, and an optional diagnostic string for logs. Trigger uses only the enum and `retryable` field for UI and control behavior.

Control semantics are:

- `STOP` is accepted from every non-`IDLE` state and returns Playback to `IDLE`;
- `PAUSE` during `LOADING`, `BUFFERING`, or `WAITING_SPEAKER` records the desired post-readiness state as paused;
- `PLAY` during those states records the desired post-readiness state as playing;
- `SEEK_MS` is accepted from `PLAYING`, `PAUSED`, `BUFFERING`, `WAITING_SPEAKER`, and `COMPLETED` when an active session exists;
- seek completion restores the play/pause intent that existed before the seek;
- replay from `COMPLETED` is implemented as `SEEK_MS` to zero followed by `PLAY`, without changing `session_id`;
- a newer accepted `LOAD_SESSION` replaces any current state immediately;
- state transitions are reported immediately, while periodic progress remains every 500 ms during active playback.

Speaker loss does not produce a fatal error: Playback enters `WAITING_SPEAKER`, freezes the media clock, and resumes according to the retained play/pause intent when the configured speaker returns. Board Link loss also does not stop the session; reconnect uses the previously decided status-resynchronization flow.
