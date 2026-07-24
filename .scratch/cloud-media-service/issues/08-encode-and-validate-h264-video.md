# 08 — Encode and validate the constrained H.264 video

**What to build:** pipe the exact ordered visualization frames into FFmpeg and produce one validated video-only fast-start MP4 that matches the authoritative audio duration and the `t5ai-h264-mp3-v1` device profile.

**Blocked by:** 07 — Render explicit deterministic visual frames.

**Status:** claimed

- [ ] FFmpeg consumes the ordered frame stream without an intermediate real-time WebM recording.
- [ ] Output contains exactly one video track and no audio, subtitle, alternate, or fragmented-MP4 tracks.
- [ ] Output is H.264 Constrained Baseline, YUV420P, progressive, 480x320, and 10 fps.
- [ ] Output contains no B frames and has a closed GOP with an IDR/keyframe interval no greater than one second.
- [ ] MP4 uses fast-start layout with `moov` before `mdat`.
- [ ] Encoded duration and frame count match the authoritative normalized duration and explicit frame-count rule without MediaRecorder tail drift.
- [ ] Validation checks codec configuration, `avcC`, track/sample tables, monotonic DTS/PTS, sample byte bounds, dimensions, frame rate, profile, pixel format, B-frame absence, and keyframe spacing.
- [ ] Video byte length, lowercase SHA-256, strong ETag value, duration, and profile metadata are available to the package publisher.
- [ ] Invalid or incomplete frame streams and FFmpeg failures remain unpublished and receive stable processing errors.
- [ ] Integration tests probe generated MP4 files and include exact-duration, fast-start, no-B-frame, keyframe-interval, and repeated-frame-hash assertions.
