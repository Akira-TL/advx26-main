# Cloud Media Service Implementation Specification

## Problem Statement

The current backend is a legacy multimedia-package service. It accepts metadata, an audio file, an STL model, and a user-provided WebM video, then stores those files under one package record. That contract no longer represents the SoundPola competition product.

The product now needs an authenticated Cloud Media Service where a user uploads one Shared Sound and retains ownership of the original audio. The cloud must perform all media preparation: inspect and repair the source, normalize audio, derive a deterministic audio-feature timeline, render the selected Sound Visualization in Headless Chromium, encode a device-ready H.264 video, generate an indexed MP3, validate the package, and publish it atomically. A fixed Trigger Board and fixed Playback Board use separate role-specific Bearer Tokens to resolve NFC content and retrieve immutable media.

The service must remain simple enough for rapid competition development. It uses one FastAPI deployment, SQLite, a local filesystem Object Store, one serial media worker, FFmpeg/FFprobe, Node.js, and Headless Chromium. It must recover durable work after process restart without requiring Redis, a message broker, PostgreSQL, S3, or a distributed worker platform.

## Solution

Replace the legacy STL/WebM package contract with a user-owned content workflow.

The backend issues long-lived opaque User Tokens and stores only token digests. An authenticated user uploads one source audio file. The service permanently stores the original source object, creates an immutable content identity and durable SQLite Processing Job, and immediately returns the content and status references.

A serial worker claims the job and performs explicit stages:

1. probe and bounded-repair the uploaded audio;
2. normalize the first at most 30 seconds into device MP3 and deterministic PCM;
3. generate and validate `audio.idx`;
4. compute a deterministic fixed-rate Audio Feature Timeline;
5. run the selected WebGL2 visualization under Headless Chromium using explicit frame index and media time;
6. pipe exactly the required 480x320 frames to FFmpeg;
7. encode fast-start H.264 Constrained Baseline video at 10 fps with no B frames and at most one-second keyframe spacing;
8. validate the complete media set and manifest;
9. atomically promote generated objects and mark the content `READY` in SQLite.

The phone-facing Sharing Interface polls owner-protected status until readiness and writes the immutable Compact Content URL to NFC. The NFC URL requires the fixed Trigger Token. Trigger receives presentation data and a normalized Playback descriptor, creates a local playback session, and sends the descriptor over BLE. Playback supplies its own fixed Playback Token for every authenticated media `GET`, `HEAD`, and Range request.

One upload creates one immutable `content_id`; READY objects are never overwritten. Regeneration creates another content item. Multiple NFC cards may contain the same Compact Content URL without cloud-side card registration.

## User Stories

1. As a new Sharing Interface installation, I want the backend to issue an opaque User Token, so that later uploads can be associated with one stable user without a full password account system.
2. As a returning user, I want my User Token to identify me, so that I can upload and inspect my own sounds.
3. As a user, I want the service to store only a digest of my User Token, so that a SQLite leak does not reveal reusable plaintext credentials.
4. As a user, I want to upload one raw audio recording or supported audio file, so that I do not need to prepare a multimedia package myself.
5. As a user, I want the service to retain my original audio permanently, so that ownership history and future regeneration remain possible.
6. As a user, I want every uploaded sound to belong to my user identity, so that another user cannot list, delete, or manage it.
7. As a user, I want upload to return quickly with a content identity and processing state, so that a 30-second cloud render does not hold an unreliable HTTP request open.
8. As a user, I want to poll my content state, so that the Sharing Interface can show upload, processing, readiness, and failure feedback.
9. As a user, I want invalid source media to fail with a stable error code, so that the client can explain that the file cannot be processed.
10. As a user, I want transient cloud failures to retry within a small bound, so that a temporary renderer or storage problem does not require immediate re-upload.
11. As a user, I want failed processing to preserve my original audio, so that a later retry or diagnosis does not lose the recording.
12. As a user, I want sounds longer than 30 seconds to keep their first 30 seconds automatically, so that no interactive trimming UI is required.
13. As a user, I want shorter sounds to retain their complete duration without padding, so that the published experience matches the recording.
14. As a user, I want the service to accept common phone audio formats, so that the Sharing Interface does not need a device-specific transcoder.
15. As a user, I want the generated sound level to be normalized and limited, so that playback is consistent and does not clip after resampling or downmixing.
16. As a user, I want the selected Sound Visualization to be generated from my sound, so that every NFC content item has an associated visual rather than a generic preset clip.
17. As a user, I want repeated processing of the same unpublished content to use the same visual seed and deterministic feature timeline, so that retries do not unexpectedly change the result.
18. As a user, I want a neutral generated label such as `声音碎片 #XXXX`, so that untitled sounds remain distinguishable without AI classification.
19. As a user, I want a Compact Content URL only after complete readiness, so that I never write a partially playable item to NFC.
20. As a user, I want to write the same Compact Content URL to multiple NFC cards, so that sharing does not require duplicate cloud media.
21. As a user, I want content sharing to preserve my ownership, so that NFC publication does not transfer management rights.
22. As a Trigger Board, I want to resolve a READY NFC URL with my fixed Trigger Token, so that the backend recognizes my device role.
23. As a Trigger Board, I want missing or wrong-role tokens rejected, so that a user or Playback credential cannot impersonate Trigger.
24. As a Trigger Board, I want the Compact Content response to include display label, duration, autoplay behavior, controls, and normalized Playback metadata, so that I can create the local session and control panel.
25. As a Trigger Board, I want the response to omit user ownership and source-object details, so that device traffic contains only playback information.
26. As a Trigger Board, I want to pass the playback descriptor over BLE without forwarding my Trigger Token, so that device credentials remain isolated.
27. As a Playback Board, I want to retrieve immutable media with my fixed Playback Token, so that the backend recognizes me separately from Trigger.
28. As a Playback Board, I want stable absolute asset endpoints, so that Trigger can send a reusable descriptor without signed-URL expiry handling.
29. As a Playback Board, I want `HEAD` and single byte ranges, so that I can parse MP4 metadata, seek, resume, and prebuffer efficiently.
30. As a Playback Board, I want exact lengths, strong ETags, and `If-Range`, so that cached and newly downloaded bytes cannot be mixed across identities.
31. As a Playback Board, I want a fast-start MP4 with `moov` before `mdat`, so that metadata is available before video sample ranges.
32. As a Playback Board, I want H.264 Constrained Baseline, YUV420P, 480x320, 10 fps, no B frames, and bounded keyframe spacing, so that the constrained decoder path can consume the video.
33. As a Playback Board, I want a separate 44.1 kHz, 128 kbps CBR MP3 with at most two channels, so that I can decode it to PCM for the Bluetooth speaker.
34. As a Playback Board, I want a validated fixed-width `audio.idx`, so that I can seek to complete MP3 frames and verify their CRC32 values.
35. As a Playback Board, I want video and audio durations to agree within the demo tolerance, so that PCM can remain the authoritative media clock.
36. As the competition operator, I want fixed Trigger and Playback Tokens supplied outside Git, so that device authentication is simple and secrets are not committed.
37. As the competition operator, I want a health endpoint independent of the media worker, so that process liveness can be checked.
38. As the competition operator, I want readiness to verify SQLite, object storage, required binaries, renderer assets, and worker availability, so that the demo does not accept uploads into a broken deployment.
39. As the competition operator, I want one media job to run at a time, so that the measured high-memory Chromium renderer cannot exhaust the host.
40. As the competition operator, I want interrupted jobs to become claimable after restart, so that a process crash does not permanently strand content.
41. As the competition operator, I want bounded processing logs and failed-job diagnostics, so that failures can be investigated without unlimited disk growth.
42. As the competition operator, I want successful temporary PCM, feature, frame, and encoder artifacts cleaned up, so that storage growth is dominated by user sources and published outputs.
43. As the competition operator, I want READY object promotion and SQLite state change coordinated atomically, so that devices never observe a partial package.
44. As the competition operator, I want the original source and immutable published objects indexed by SQLite metadata, so that storage can be audited and cleaned safely.
45. As a developer, I want media subprocesses behind explicit adapters, so that API and job-state tests can use deterministic fakes while integration tests exercise installed binaries.
46. As a developer, I want the Object Store behind a narrow interface, so that local filesystem storage can later be replaced without rewriting domain logic.
47. As a developer, I want the renderer to consume an explicit Audio Feature Timeline and frame timestamp, so that production output does not depend on `requestAnimationFrame`, `performance.now()`, live audio sampling, or MediaRecorder.
48. As a developer, I want exact frame-count and repeated-render tests, so that duration and determinism regressions are detected before device integration.
49. As a developer, I want API behavior tested through FastAPI's ASGI boundary, so that tests assert external contracts rather than route implementation details.
50. As a developer, I want the obsolete metadata/STL/WebM and ZIP bundle paths removed, so that there is one canonical media model and no accidental legacy dependency.

## Implementation Decisions

### Service boundaries

- Keep one FastAPI application and one deployable backend process for the competition build.
- Split the current monolithic route implementation into deep modules with narrow interfaces: authentication, content repository, processing jobs, Object Store, media tools, renderer, publisher, and HTTP asset delivery.
- Run one in-process serial worker. Do not introduce a separate message broker or worker deployment.
- Preserve simple health and readiness endpoints, but readiness must include required runtime dependencies rather than only database and directory writes.

### Identity and authorization

- User identities are minimal records with opaque long-lived User Tokens. Plaintext is returned only on issuance; SQLite stores a cryptographic digest and token metadata.
- A User Token authorizes source upload, owner status/list operations, and owner lifecycle actions.
- Trigger and Playback use two fixed deployment tokens mapped to stable role identities. They are loaded from Git-ignored configuration and compared in constant time.
- Trigger may resolve READY Compact Content documents. Playback may retrieve the playback descriptor and immutable media objects. Device tokens cannot upload or manage user content.
- User Tokens cannot call device-only routes. Trigger and Playback Tokens are never forwarded to each other over BLE.
- No password login, refresh tokens, JWT signing, device registration, pairing records, signed URLs, or per-session cloud grants are added.

### Content and persistence model

- Use separate SQLite concepts for Users, User Tokens, Contents, Processing Jobs, Media Objects, and any bounded processing-attempt history.
- One authenticated upload creates one immutable `content_id` and one owner relationship.
- The original upload is permanently stored as an Object Store object and referenced from SQLite.
- READY content has exactly one published `video.mp4`, `audio.mp3`, `audio.idx`, and `manifest.json` set.
- READY outputs are immutable. Regeneration creates another content identity.
- Physical NFC cards and UIDs are not persisted.
- The first Object Store implementation maps opaque object keys to a server-local filesystem hierarchy. Domain and route code never construct arbitrary absolute paths.

### Processing jobs

- Upload creates a durable job and returns before media preparation completes.
- Jobs use explicit stages and timestamps. Claiming must prevent two workers from processing the same job.
- An interrupted claimed job becomes reclaimable after a bounded lease or startup recovery rule.
- Invalid input errors are terminal. Renderer, encoder, and storage failures have a small fixed retry budget.
- Only one job executes at a time initially because the prototype measured approximately 1.23 GiB peak Chromium process-tree RSS under SwiftShader.
- Source objects remain after failure. Successful temporary artifacts are cleaned; failed diagnostic artifacts are retained only within configured bounds.

### Audio input and normalization

- Accept WAV, MP3, M4A/AAC, Ogg/Opus, and WebM audio up to 50 MiB.
- Inspect bytes with FFprobe. Filename and client MIME type are advisory.
- Select the first decodable, non-encrypted audio stream. Reject media with no supported stream or failed bounded repair.
- Permanently retain the original bytes.
- Normalize at most the first 30 seconds.
- Generate 44.1 kHz, 128 kbps CBR MP3. Preserve mono and downmix two or more input channels to stereo. Limit peaks near -1 dBFS.
- Generate deterministic PCM for feature extraction. The normalized decoded duration is authoritative.

### MP3 index

- Publish the `AIX1` fixed-width little-endian format already decided in Wayfinder.
- The 16-byte header declares version 1, 16-byte records, record count, and zero reserved bytes.
- Each record contains decoded PCM sample position, MP3 byte offset, complete frame byte length, and CRC32 of that frame.
- Validate monotonic sample positions, complete in-bounds non-overlapping frames, CRC32 values, and indexed duration before publication.

### Deterministic visualization rendering

- Reuse the existing WebGL2 shaders and visual implementation from `Sound-Visualization-Kaleidoscope-effect/particle-field`.
- Do not use microphone access, real-time audio playback, `requestAnimationFrame` time, `performance.now()` time, Canvas `captureStream`, or MediaRecorder in the production pipeline.
- Compute a deterministic fixed-rate Audio Feature Timeline from normalized PCM. The timeline must cover every rendered frame and encode the same feature vocabulary needed by the renderer.
- Derive and persist a visual seed from `content_id`.
- Adapt the renderer to accept explicit frame index, exact media time, feature sample, and seed.
- Render exactly `ceil(duration_seconds * 10)` frames at timestamps `frame_index / 10` on a 480x320 canvas.
- Export ordered raw or lossless frames from Chromium and pipe them directly to FFmpeg.
- A retry using the same normalized audio, feature timeline, renderer version, and seed must reproduce identical decoded frames.
- The prototype branch `prototype/headless-cloud-render` at commit `38dd584` is the primary source proving Headless WebGL viability and disproving MediaRecorder determinism.

### Video encoding and validation

- Encode one video-only non-fragmented MP4.
- Use H.264 Constrained Baseline, YUV420P, progressive 480x320 frames, 10 fps, no B frames, closed GOP, IDR/keyframe spacing at most one second, and fast-start layout.
- Trim the frame stream only to the authoritative audio duration; do not loop a preset clip.
- Validate one supported video track, box ordering, `avcC`, sample tables, timestamps, sample bounds, frame rate, dimensions, profile, pixel format, B-frame absence, and keyframe spacing.
- Initial bitrate tuning remains adjustable without changing the `t5ai-h264-mp3-v1` contract.

### Atomic publication

- Processing uses a private staging keyspace.
- Before publication validate manifest schema, content identity, profile, duration agreement, object lengths, SHA-256 values, MP4 constraints, MP3 index integrity, and all generated URLs.
- Promote the complete generated object set into the immutable content keyspace with an atomic filesystem rename where supported.
- In one SQLite transaction, insert final Media Object metadata and change the content state to `READY`.
- Device routes require `READY`; they never expose staged or FAILED outputs.

### HTTP contracts

- Dynamic User Token issuance, upload, list, status, retry, and lifecycle responses use `Cache-Control: no-store`.
- Compact Content requires the Trigger Token and returns the complete Trigger-facing document directly with HTTP 200 only for READY content.
- Playback media endpoints require the Playback Token.
- Immutable authenticated responses use strong SHA-derived ETags, exact Content-Length, `Cache-Control: private, max-age=31536000, immutable`, and `Vary: Authorization`.
- Media endpoints support `GET`, `HEAD`, one byte range, `If-Range`, `206`, and correct `416` behavior. Multipart ranges are not required.
- FastAPI proxies bytes from the local Object Store. Public direct object paths and signed URLs are not required.

### Legacy migration

- Remove required metadata JSON, creator metadata embedded in upload, user video, STL, STL validation, WebM-only validation, model-file routes, legacy package listing semantics, ZIP bundles, and old Swagger artifacts.
- Preserve only behavior that remains part of the new contract, such as health/readiness concepts, bounded streaming primitives, and useful test infrastructure.
- Existing package data does not require an automatic production migration for the competition repository unless real retained data is discovered before implementation.

## Testing Decisions

- The primary test seam is the FastAPI ASGI boundary using isolated temporary SQLite and Object Store directories. Tests submit HTTP requests and assert status, response schema, authorization, ownership, state transitions, cache headers, ETags, Range behavior, and immutable bytes.
- Authentication tests cover issued User Tokens, digest lookup, unknown tokens, disabled tokens when supported, and strict role rejection between USER, TRIGGER, and PLAYBACK routes.
- Repository and job tests exercise SQLite transactions, claim exclusivity, restart recovery, retry classification, and atomic READY visibility without asserting SQL implementation details.
- Object Store contract tests run against the filesystem adapter and verify opaque keys, bounded reads, stat metadata, staging promotion, deletion, and traversal rejection.
- Media adapter unit tests use deterministic fake subprocess results at the highest seam possible. They verify command inputs and interpreted outputs, not every low-level command-building helper.
- FFprobe/FFmpeg integration tests use small generated fixtures and run only when required binaries are available. They verify accepted formats, rejection, duration truncation, MP3 parameters, index integrity, MP4 profile, box order, frame rate, no B frames, keyframe interval, and duration agreement.
- Renderer integration tests run the built headless renderer with a short deterministic PCM/feature fixture. Two executions must produce identical decoded frame hashes and exact frame counts.
- Publication tests inject failures before and during promotion and assert that device endpoints never observe partial outputs.
- HTTP Range tests cover full GET, HEAD, prefix/open/suffix ranges, malformed ranges, unsatisfiable ranges, matching and mismatching `If-Range`, exact Content-Range, and no cross-identity concatenation.
- Existing tests for the legacy STL/WebM contract are replaced rather than mechanically retained.
- Every implementation ticket ends with syntax/static checks, targeted tests, the complete backend test suite, and a Standards plus Spec review before commit.

## Out of Scope

- Passwords, email verification, social login, account recovery, refresh tokens, organizations, profiles, or billing.
- Production token rotation, rate limits, abuse prevention, device enrollment, attestation, cloud pairing, signed URLs, or hostile-network hardening.
- PostgreSQL, Redis, a message broker, distributed workers, Kubernetes, or required S3-compatible storage.
- User-uploaded video, Preset Video pools, STL models, local 3D rendering, or ZIP multimedia bundles.
- Multiple selectable visualization themes, AI-generated visual styles, or user-authored video controls.
- Media longer than 30 seconds, arbitrary internet media playback, or multi-item playback queues.
- Trigger firmware, PN532 electrical integration, Playback decoder implementation, Bluetooth speaker pairing, or the external Sharing Interface implementation.
- Physical NFC card UID registration or cloud ownership of individual cards.
- General-purpose administrative UI.

## Further Notes

- The current backend `develop/akira` baseline is clean and its legacy three-test suite passes, but those tests describe the obsolete contract.
- Root planning commit `923c080` records the corrected cloud, ownership, and authentication decisions. Commit `0e9c7b8` records the headless-render prototype verdict.
- The visualization prototype is intentionally retained only on `Sound-Visualization-Kaleidoscope-effect:prototype/headless-cloud-render` at commit `38dd584`. Production implementation should port the validated decisions, not merge the real-time MediaRecorder prototype wholesale.
- The user-facing mobile client remains external, but the backend must expose enough contract fixtures and OpenAPI documentation for that client to integrate later.
