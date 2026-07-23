# Decide atomic Media Package publication

Type: grilling
Status: resolved
Blocked by: 08, 25

## Question

When does a prepared revision become visible through its Compact Content URL?

## Answer

The Cloud Media Service prepares every revision in a private staging area. Individual generated files are not addressable through public asset URLs and the Compact Content URL does not resolve to a playable descriptor while preparation is incomplete.

Before publication, the service validates at least:

- `manifest.json` schema and the `t5ai-h264-mp3-v1` profile identifier;
- video/audio duration agreement and the 30-second limit;
- exact file lengths and SHA-256 values;
- `video.mp4` has fast-start layout, exactly one supported H.264 track, supported profile/pixel format/dimensions/frame rate, no B frames, bounded GOP/IDR spacing, valid `avcC`, valid sample tables, monotonic timestamps, and in-bounds sample ranges;
- `audio.idx` magic, version, record size, monotonic sample positions, byte bounds, non-overlap, record CRC32 values, and final indexed duration;
- all manifest URLs and revision identities refer to the same staged revision.

After every validation passes, one database transaction plus an atomic storage rename/promotion changes the revision to `READY` and exposes its immutable assets and Compact Content URL together. Playback can therefore observe either no published revision or one complete revision, never a partially visible package.

A failed staging attempt enters `FAILED`, remains inaccessible to Playback, and may be cleaned up later. Automatic or manual retry may reuse an unpublished staging revision identity, but it must not mutate an already published revision. Any change to published media creates a new immutable revision.
