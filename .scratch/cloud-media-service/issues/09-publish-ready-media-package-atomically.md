# 09 — Publish the complete READY media package atomically

**What to build:** validate the independently completed audio and video chains, create one immutable device manifest, promote the complete generated object set, and expose the content as READY in one coordinated publication step.

**Blocked by:** 05 — Generate and validate the indexed MP3 asset; 08 — Encode and validate the constrained H.264 video.

**Status:** resolved

**Resolved by:** backend commit `5157397 feat(publish): 原子发布完整媒体包`

- [x] Publication requires and validates video, audio, index, duration, label, and integrity metadata.
- [x] Manifest schema 1 uses immutable content identity and `t5ai-h264-mp3-v1` descriptors.
- [x] Audio/video duration and profile compatibility are checked before publication.
- [x] Every URL, key, length, SHA-256, ETag, codec field, and index version is internally consistent.
- [x] Generated files remain private until complete validation.
- [x] The complete object directory is promoted atomically before one SQLite READY transaction.
- [x] Promotion/database failures leave device-visible state non-READY and are safely retryable.
- [x] Final objects are immutable and cannot be replaced by differing bytes.
- [x] Owner and original source relationships remain intact.
- [x] Failure-injection tests cover validation, promotion, post-promotion database failure, and retry.
