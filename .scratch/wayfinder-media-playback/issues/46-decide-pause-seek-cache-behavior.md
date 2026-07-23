# Decide pause, seek, and cache behavior

Type: grilling
Status: resolved
Blocked by: 11, 16, 45

## Question

What should Playback do with network reads and buffered media while paused, replaying, or seeking outside the currently buffered range?

## Answer

Playback uses bounded in-memory prefetching and does not require TF card storage or whole-clip persistence.

When paused:

- the media clock stops;
- Playback continues HTTP Range reads only until the compressed MP4/H.264 sample queue, compressed MP3 input buffer, decoded video queue, and decoded PCM ring reach configured high-water marks;
- network reads then stop until playback resumes or a seek invalidates the buffered range;
- BLE status and command handling remain active.

A seek inside the buffered window may reuse already parsed MP4 metadata and buffered samples when the required decoder dependency chain is intact. A seek outside that safe window:

1. cancels obsolete pending Range requests;
2. clears decoded video, H.264 reference state, MP3, and PCM queues;
3. selects the closest MP4 sync sample at or before the target and the corresponding MP3 position from `audio.idx`;
4. starts new HTTP Range requests under the same `session_id`;
5. reapplies MP4 codec initialization data to the H.264 decoder;
6. decodes video forward from the sync sample and MP3 from its preroll window;
7. suppresses output until the requested position and rebuilds the normal startup prebuffer.

Seeking while playing enters a temporary buffering state and resumes automatically after the target prebuffer is ready. Seeking while paused fills the target prebuffer but remains paused.

Replay reuses valid beginning-of-session data still resident in memory. Missing data is fetched again through immutable Range requests. Replay does not require a new content revision or a new session identity unless Trigger explicitly issues a new `LOAD_SESSION`.

`STOP` releases MP4 metadata, compressed and decoded buffers, and decoder state; it returns Playback to the idle screen while Trigger retains the content panel.
