# 04 — Probe, repair, and normalize source audio

**What to build:** turn one claimed owned source upload into validated normalized MP3 and deterministic PCM staging artifacts while preserving the original bytes and reporting stable terminal or retryable failures.

**Blocked by:** 03 — Run durable processing jobs with restart recovery.

**Status:** resolved

**Resolved by:** backend commit `94d000e feat(audio): 实现云端音频探测与归一化`

- [x] FFprobe inspects uploaded bytes rather than trusting filename extension or client MIME type.
- [x] WAV, MP3, M4A/AAC, Ogg/Opus, and WebM audio fixtures are accepted when they contain a decodable non-encrypted audio stream.
- [x] Media with no audio stream, unparseable bytes, or failed bounded repair enters a stable terminal failure state; encrypted tracks are explicitly rejected by the adapter model.
- [x] The first supported audio stream is retained for at most 30 seconds; shorter decoded audio is not padded.
- [x] Generated `audio.mp3` is 44.1 kHz, 128 kbps CBR, mono for mono input and stereo for input with two or more channels, with peak limiting near -1 dBFS.
- [x] Deterministic S16LE PCM suitable for feature extraction is generated from the same authoritative normalized timeline.
- [x] The measured normalized duration is persisted and becomes the authoritative content duration.
- [x] Original source bytes remain unchanged and permanently addressable through owner metadata after success or failure.
- [x] FFmpeg and FFprobe execution is behind an adapter with bounded runtime, captured diagnostics, and no shell interpolation.
- [x] Binary-backed integration tests verify format acceptance, 30-second truncation, channel behavior, MP3 parameters, decoded duration, limiter behavior, deterministic retry, and stable failure classification.
