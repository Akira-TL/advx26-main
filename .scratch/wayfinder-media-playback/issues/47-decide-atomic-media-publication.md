# Decide atomic Media Package publication

Type: grilling
Status: resolved
Blocked by: 08, 25, 56

## Question

When does a processed content item become visible through its Compact Content URL and Playback media endpoints?

## Answer

The Cloud Media Service creates the user-owned content record and permanently stores its original source audio before processing, but generated media remains in a private staging keyspace while the SQLite Processing Job runs. Trigger and Playback endpoints reject any content whose state is not `READY`.

Before publication, the service validates at least:

- `manifest.json` schema, immutable `content_id`, and `t5ai-h264-mp3-v1` profile;
- generated video and normalized audio duration agreement and the 30-second limit;
- exact object lengths and lowercase SHA-256 values;
- `video.mp4` fast-start layout, one supported H.264 track, supported profile, pixel format, dimensions and frame rate, no B frames, bounded GOP/IDR spacing, valid `avcC`, valid sample tables, monotonic timestamps, and in-bounds sample ranges;
- `audio.idx` magic, version, record size, monotonic sample positions, byte bounds, non-overlap, record CRC32 values, and final indexed duration;
- every manifest and device URL refers to the same content identity and final object set.

After validation, generated objects are promoted from staging into the immutable content keyspace with an atomic filesystem rename where possible. A SQLite transaction records the final object metadata and changes the content state to `READY`. The Compact Content and media routes consult that state, so devices observe either no playable package or the complete package, never a partially generated set.

A failed job enters `FAILED` and remains inaccessible to Trigger and Playback. Its original source audio remains owned and stored. Bounded retry may reuse the unpublished content identity and deterministic render seed. Once the content is READY, its generated objects are never overwritten; a requested regeneration creates a new content item.
