# Decide pause, seek, and cache behavior

Type: grilling
Status: resolved
Blocked by: 11, 16, 45

## Question

What should Playback do with network reads and buffered media while paused, replaying, or seeking outside the currently buffered range?

Decide whether pause continues bounded prefetching, whether replay reuses only currently buffered data or persists the full clip, and whether an out-of-range seek reopens HTTP Range requests without changing the current session identity.

## Answer

Playback uses bounded in-memory prefetching and does not require TF card storage or whole-clip persistence.

When paused:

- the media clock stops;
- Playback continues HTTP Range reads only until the compressed MP3 input buffer, decoded PCM ring, and JPEG frame queue reach their configured high-water marks;
- network reads then stop until playback resumes or a seek invalidates the buffered range;
- BLE status and command handling remain active.

A seek inside the buffered window repositions within existing data where safe. A seek outside that window:

1. cancels obsolete pending Range requests;
2. clears JPEG decode/output queues and MP3/PCM decoder state;
3. selects video and audio offsets through `video.idx` and `audio.idx`;
4. starts new HTTP Range requests under the same `session_id`;
5. rebuilds the normal startup prebuffer before reporting the new position ready.

Seeking while playing enters a temporary buffering state and resumes automatically after the target prebuffer is ready. Seeking while paused fills the target prebuffer but remains paused.

Replay reuses any valid beginning-of-session data still resident in memory. Missing data is fetched again through immutable Range requests. Replay does not require a new content revision or a new session identity unless Trigger explicitly issues a new `LOAD_SESSION`.

`STOP` releases media buffers and decoder state, returns Playback to the idle screen, and leaves Trigger responsible for retaining the content panel and issuing a later replay/load command.
