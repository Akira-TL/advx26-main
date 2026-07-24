# 08 — Encode and validate the constrained H.264 video

**What to build:** pipe the exact ordered visualization frames into FFmpeg and produce one validated video-only fast-start MP4 that matches the authoritative audio duration and the `t5ai-h264-mp3-v1` device profile.

**Blocked by:** 07 — Render explicit deterministic visual frames.

**Status:** resolved

**Resolved by:** backend commit `fb6b4e7 feat(video): 编码并校验设备兼容H264视频`

- [x] FFmpeg consumes ordered PNG frames without WebM or MediaRecorder.
- [x] Output contains exactly one video track and no alternate media tracks.
- [x] Output is H.264 Constrained Baseline, YUV420P, progressive, 480x320, and 10 fps.
- [x] Output contains no B frames and uses a closed GOP with at most ten frames per GOP.
- [x] MP4 uses fast-start layout with `moov` before `mdat`, no `moof`, and no edit list.
- [x] Frame count is `ceil(duration×10)`; video covers audio with less than one frame of tail.
- [x] Validation checks `avcC`, sample tables, monotonic DTS/PTS, byte bounds, dimensions, rates, profile, pixel format, B-frame absence, and keyframe spacing.
- [x] Video length, SHA-256, ETag, duration, frame count, and profile metadata are available to publication.
- [x] Invalid frame streams and FFmpeg failures remain private with stable errors.
- [x] Integration tests verify fast-start, no B frames, keyframe interval, tail tolerance, and deterministic video SHA-256.
