# 05 — Generate and validate the indexed MP3 asset

**What to build:** convert the normalized MP3 into a complete seekable audio asset pair whose fixed-width index lets Playback locate and verify full MP3 frames against the authoritative decoded timeline.

**Blocked by:** 04 — Probe, repair, and normalize source audio.

**Status:** claimed

- [ ] The worker parses every complete MP3 frame in normalized `audio.mp3` without relying on approximate bitrate arithmetic.
- [ ] Generated `audio.idx` begins with the exact `AIX1` version-1 header and uses 16-byte little-endian records.
- [ ] Every record contains decoded PCM sample position, MP3 byte offset, complete frame length, and CRC32 of the indexed frame bytes.
- [ ] Sample positions and byte offsets are monotonic; frames are complete, non-overlapping, and fully in bounds.
- [ ] Indexed duration agrees with the authoritative normalized duration within one MP3-frame tolerance.
- [ ] Corrupted MP3 bytes, invalid headers, out-of-bounds records, overlapping records, CRC mismatch, and unsupported index versions are rejected before publication.
- [ ] Audio object length, lowercase SHA-256, strong ETag value, sample rate, bitrate, channel count, index version, and index object metadata are available to the future manifest publisher.
- [ ] Unit tests use known MP3 frame fixtures, and integration tests verify that FFmpeg-generated normalized MP3 produces a valid index accepted by the same validator.
