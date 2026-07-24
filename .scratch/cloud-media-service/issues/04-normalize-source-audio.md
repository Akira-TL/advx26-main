# 04 — Probe, repair, and normalize source audio

**What to build:** turn one claimed owned source upload into validated normalized MP3 and deterministic PCM staging artifacts while preserving the original bytes and reporting stable terminal or retryable failures.

**Blocked by:** 03 — Run durable processing jobs with restart recovery.

**Status:** claimed

- [ ] FFprobe inspects uploaded bytes rather than trusting filename extension or client MIME type.
- [ ] WAV, MP3, M4A/AAC, Ogg/Opus, and WebM audio fixtures are accepted when they contain a decodable non-encrypted audio stream.
- [ ] Media with no audio stream, encrypted audio, unparseable bytes, or failed bounded repair enters a stable terminal failure state.
- [ ] The first valid audio stream is retained for at most 30 seconds; shorter decoded audio is not padded.
- [ ] Generated `audio.mp3` is 44.1 kHz, 128 kbps CBR, mono for mono input and stereo for input with two or more channels, with peak limiting near -1 dBFS.
- [ ] Deterministic PCM suitable for feature extraction is generated from the same authoritative normalized timeline.
- [ ] The measured normalized duration is persisted and becomes the authoritative content duration.
- [ ] Original source bytes remain unchanged and permanently addressable through owner metadata after success or failure.
- [ ] FFmpeg and FFprobe execution is behind an adapter with bounded runtime, captured diagnostics, and no shell interpolation.
- [ ] Small binary-backed integration tests verify format acceptance, 30-second truncation, channel behavior, MP3 parameters, decoded duration, and stable failure classification.
