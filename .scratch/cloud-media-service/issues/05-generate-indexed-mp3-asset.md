# 05 — Generate and validate the indexed MP3 asset

**What to build:** convert the normalized MP3 into a complete seekable audio asset pair whose fixed-width index lets Playback locate and verify full MP3 frames against the authoritative decoded timeline.

**Blocked by:** 04 — Probe, repair, and normalize source audio.

**Status:** resolved

**Resolved by:** backend commit `c1c7c8a feat(audio): 生成并校验MP3帧索引`

- [x] Every complete MP3 frame is parsed without approximate bitrate arithmetic.
- [x] Generated `audio.idx` begins with the exact `AIX1` version-1 header and uses 16-byte little-endian records.
- [x] Every record contains decoded PCM sample position, MP3 byte offset, complete frame length, and CRC32.
- [x] Sample positions and byte offsets are monotonic; frames are complete, non-overlapping, and in bounds.
- [x] Indexed duration agrees with the authoritative normalized duration within one rounded-up MP3-frame tolerance.
- [x] Corrupted MP3 bytes, invalid headers, out-of-bounds records, overlapping records, CRC mismatch, and unsupported versions are rejected.
- [x] Audio and index length, SHA-256, strong ETag, sample rate, bitrate, channels, version, and record count are available to the future publisher.
- [x] Synthetic frame tests and FFmpeg-generated normalized MP3 integration tests use the same validator.
