# Wayfinder Map: T5AI cloud media playback

## Destination

Produce an implementation-ready technical specification, ADR set, and dependency-ordered development tickets for a competition demo where an authenticated user uploads a Shared Sound; the Cloud Media Service normalizes it, renders a matching visual with `Sound-Visualization-Kaleidoscope-effect`, publishes an immutable device-ready media package, and one fixed T5AI Trigger Board reads its NFC URL and remotely controls one fixed T5AI Playback Board over BLE while Playback renders video and sends decoded audio to a Bluetooth Speaker.

## Notes

- Planning artifacts use the local Markdown tracker under this directory.
- The user prefers batching genuinely independent questions in one round.
- Both managed boards use the same Tuya T5AI board and attached 3.5-inch ILI9488/GT1151 display.
- The active repository root is `/home/akira/Projects/advx26`.
- Any additional connected board remains unmanaged until the user explicitly assigns it; it must not be flashed, opened over serial, or inferred as Trigger/Playback.
- `backend/` is a Git submodule containing the FastAPI Cloud Media Service.
- `Sound-Visualization-Kaleidoscope-effect/particle-field` is the selected WebGL2 visualization source used for cloud video rendering.
- Competition speed is more important than production-grade account recovery, device enrollment, token rotation, distributed processing, or hostile-network hardening.
- The backend remains one service using SQLite and a simple local filesystem Object Store for the competition build.
- Wayfinder produces decisions, not the final implementation. When the remaining prototype frontier clears, hand the map to specification and ticket generation.

## Decisions so far

- [Define the two board roles](issues/01-define-board-roles.md) — Use identical T5AI hardware with fixed Trigger Board and Playback Board roles.
- [Separate the two Bluetooth links](issues/02-separate-bluetooth-links.md) — BLE is a bidirectional control/status link between boards; Bluetooth audio is a separate link from Playback Board to speaker.
- [Set the competition security boundary](issues/03-set-security-boundary.md) — Use backend-issued User Tokens for upload ownership plus fixed role-separated Trigger and Playback Bearer Tokens.
- [Make the backend the media contract owner](issues/04-backend-contract-owner.md) — FastAPI owns user identity, upload, media processing, object publication, manifests, and authenticated delivery; STL, user video, local 3D rendering, and Preset Video pools leave the product path.
- [Make the Trigger Board the playback authority](issues/05-make-trigger-board-playback-authority.md) — NFC starts a session owned by Trigger, which controls playback and receives acknowledgements, state, progress, completion, and errors.
- [Set the media duration and synchronization target](issues/06-set-media-duration-and-sync-target.md) — Audio and video have equal duration, are at most 30 seconds, and may differ by roughly 100–300 ms in the demo.
- [Set replacement behavior and start latency](issues/07-set-replacement-and-start-latency.md) — New content interrupts current playback; target playback start is 3–5 seconds after a valid request.
- [Use durable SQLite media preparation](issues/08-use-synchronous-cloud-transcoding.md) — Upload returns after creating a durable Processing Job; one in-process worker performs normalization, headless visualization rendering, encoding, indexing, validation, and publication.
- [Fix the Bluetooth speaker target](issues/09-fix-the-speaker-target.md) — The first demo auto-connects to one configured speaker instead of implementing a general picker.
- [Resolve NFC into an authenticated Playback Session](issues/10-decide-nfc-session-resolution.md) — NFC stores only a Compact Content URL; Trigger resolves it with its fixed token and Playback downloads with its own token.
- [Define the Board Link command and report model](issues/11-decide-board-link-command-model.md) — Support load, play, pause, seek, stop, correlated ACK/NACK, immediate state/error reports, and 500 ms progress reports.
- [Keep all interaction on the Trigger Board](issues/12-decide-playback-interaction-semantics.md) — Playback is output-only; Trigger shows playback information, progress, controls, and feedback.
- [Research simultaneous Board Link and Speaker Link capability](issues/13-research-dual-bluetooth-capability.md) — T5AI exposes BLE GATT and A2DP Source together, but sustained coexistence must pass an early hardware prototype.
- [Select the device-ready MP4/H.264 and MP3 profile](issues/14-research-media-decode-capability.md) — Use constrained `video.mp4` plus independent `audio.mp3`/`audio.idx` under `t5ai-h264-mp3-v1`.
- [Define the device media manifest contract](issues/15-decide-media-manifest-contract.md) — Compact Content JSON separates Trigger presentation from a normalized Playback descriptor; one immutable `content_id` replaces a separate public revision number.
- [Use partial prebuffering with a PCM-derived media clock](issues/16-decide-buffering-and-sync-architecture.md) — Parse MP4 metadata first, prebuffer PCM and bounded video queues, decode in dependency order, and never stall audio for video.
- [Bound media corruption and decoder recovery](issues/17-decide-disconnect-and-recovery.md) — Recover H.264 at the next valid IDR, resynchronize isolated MP3 failures with timeline-preserving silence, and distinguish network failure from immutable content corruption.
- [Define end-to-end demo acceptance](issues/18-define-demo-acceptance.md) — Final acceptance remains blocked by hardware prototypes and complete backend publication.
- [Prototype simultaneous BLE and A2DP](issues/19-prototype-dual-bluetooth-coexistence.md) — Validate sustained Board Link and Speaker Link coexistence on physical hardware.
- [Prototype H.264 and indexed MP3 playback](issues/20-prototype-h264-mp3-playback.md) — Validate the selected media profile on physical Playback hardware.
- [Use a landscape Playback Board profile](issues/21-decide-playback-display-orientation.md) — Cloud output is 480x320 so the device does not rotate frames at runtime.
- [Define the Trigger Control Panel contract](issues/22-decide-trigger-control-panel-contract.md) — Show generated identity, duration, state, progress, standard controls, and feedback without creator-authored metadata or custom actions.
- [Define automatic playback lifecycle](issues/23-decide-playback-lifecycle-behavior.md) — NFC resolution auto-loads and auto-plays; completion holds the final frame and exposes replay on Trigger.
- [Use an authenticated audio-only sharing source](issues/24-decide-upload-source-contract.md) — A User Token-authenticated upload submits one raw sound; cloud rendering produces its video and normalized audio package.
- [Return immutable authenticated Compact Content JSON](issues/25-decide-compact-url-contract.md) — The copied NFC URL identifies one READY content item, requires Trigger authentication, and never exposes partial or mutable content.
- [Replace Preset Video selection with deterministic cloud rendering](issues/26-decide-preset-video-selection.md) — Each content uses the selected visualization renderer with a seed derived from `content_id`.
- [Generate neutral labels for untitled sounds](issues/27-decide-untitled-sound-label.md) — Trigger shows `声音碎片 #XXXX` plus recording/submission time when available.
- [Normalize Shared Sounds to at most 30 seconds](issues/28-decide-audio-duration-policy.md) — Retain at most the first 30 seconds and render the visual against the exact normalized timeline.
- [Use a User Token-based phone sharing and NFC flow](issues/29-decide-sharing-and-nfc-publishing-flow.md) — The phone persists its User Token, uploads owned audio, polls readiness, and writes the immutable Compact Content URL to NFC.
- [Keep both board applications in one repository](issues/30-decide-firmware-repository-layout.md) — Use shared protocol/domain code with separate Trigger and Playback applications.
- [Treat PN532 as an external NDEF URL source](issues/31-decide-trigger-nfc-hardware.md) — This workstream consumes a URL callback and does not own PN532 wiring, pins, or transport integration.
- [Use compile-time demo networking](issues/32-decide-demo-network-provisioning.md) — Both boards use local Git-ignored Wi-Fi, backend base URL, and fixed device-token configuration.
- [Pair one fixed speaker once](issues/33-decide-fixed-speaker-identity.md) — Playback persists one speaker pairing record and reconnects only to that A2DP Sink.
- [Keep the Sharing Interface external](issues/34-decide-sharing-interface-scope.md) — This repository owns User Token and backend contracts; mobile recording, token persistence, status UX, and NFC writing remain external.
- [Discover Playback by Board Link Service UUID](issues/35-decide-board-link-discovery.md) — Trigger auto-connects at boot and continuously retries without a user-facing picker.
- [Use JSON messages over a binary BLE envelope](issues/36-decide-board-link-message-encoding.md) — One write-with-response Command characteristic and one Notify Report characteristic share the protocol.
- [Bound and fragment Board Link messages](issues/37-decide-board-link-fragmentation-limits.md) — Reassembled JSON is capped at 4 KiB, individual URLs at 1024 bytes, and incomplete assemblies expire after five seconds.
- [Serialize commands with retry-safe sequence IDs](issues/38-decide-command-retry-and-idempotency.md) — One command is in flight, ACK timeout is one second, retries are capped at three, and duplicate execution is suppressed.
- [Resynchronize sessions after reconnect](issues/39-decide-board-link-handshake-and-resync.md) — HELLO negotiates version/capabilities; GET_STATUS adopts live state or reloads a lost session at the last confirmed position.
- [Wait for the fixed speaker instead of playing silently](issues/40-decide-speaker-unavailable-playback.md) — Speaker loss pauses the media clock and resumes automatically after the configured sink reconnects.
- [Assign stable physical board identities](issues/41-assign-physical-board-identities.md) — `5AAE167197` is Playback and `5AAE167460` is Trigger; all other connected boards remain unmanaged.
- [Defer the physical speaker identity](issues/42-identify-prototype-speaker.md) — No prototype speaker is currently available, so hardware media work must not substitute another Bluetooth device.
- [Set the dual-Bluetooth prototype acceptance threshold](issues/43-decide-dual-bluetooth-prototype-acceptance.md) — Require a 60-second zero-disconnect run, 500 ms reports, 20 timely command/ACK exchanges, and no underruns or audible interruption.
- [Provide the fixed prototype speaker](issues/44-provide-prototype-speaker.md) — Physical speaker availability remains a user-supplied hardware blocker.
- [Use MP4 sample tables plus an indexed MP3 asset](issues/45-decide-media-index-and-integrity.md) — Video timing and ranges come from MP4 metadata; MP3 uses fixed-width `audio.idx` with CRC32 records.
- [Use bounded in-memory pause, seek, and replay caching](issues/46-decide-pause-seek-cache-behavior.md) — Pause and seek use bounded queues and immutable Range reads without changing the content identity.
- [Publish Media Packages atomically](issues/47-decide-atomic-media-publication.md) — Validate private staged outputs, promote the object set, then transactionally mark the content READY in SQLite.
- [Freeze Board Link UUIDs and the v1 fragment header](issues/48-decide-board-link-wire-constants.md) — Use stable private UUIDs and a fixed 16-byte little-endian envelope.
- [Normalize common phone audio into bounded MP3](issues/49-decide-source-audio-normalization.md) — Inspect and repair common source formats, retain the original upload, normalize to 44.1 kHz 128 kbps CBR MP3, and drive rendering from that timeline.
- [Use authenticated immutable byte-range HTTP transport](issues/50-decide-immutable-http-transport.md) — Role-protected content exposes strong ETags, private immutable caching, HEAD, single byte ranges, `If-Range`, and exact lengths.
- [Freeze Playback states and errors](issues/51-decide-playback-state-and-errors.md) — Use nine stable states, eight stable fatal error categories, immediate state reports, retained intent, and enum-driven Trigger behavior.
- [Freeze the Board Link JSON body schema](issues/52-decide-board-link-json-schema.md) — Strict versioned messages carry immutable `content_id` without a separate revision field.
- [Define User Token issuance and content ownership](issues/53-define-user-token-and-content-ownership.md) — Backend-issued opaque User Tokens identify owners for upload, listing, status, and management while shared NFC playback preserves ownership.
- [Define fixed Trigger and Playback token roles](issues/54-define-fixed-device-token-roles.md) — Two Git-ignored fixed Bearer Tokens identify the only Trigger and Playback and authorize only their required routes.
- [Render the selected visualization in the cloud](issues/55-render-cloud-visualization.md) — Adapt the existing WebGL2 renderer for file audio, deterministic seed, headless 480x320 rendering, and FFmpeg H.264 output.
- [Use SQLite media jobs and local Object Storage](issues/56-use-sqlite-media-jobs-and-local-object-storage.md) — One serial in-process worker persists stages in SQLite and stores permanent source and READY objects behind a filesystem Object Store interface.

## Active decision frontier

- [Prototype headless cloud visualization rendering](issues/57-prototype-headless-cloud-rendering.md) — Prove that the current WebGL2 project can consume a supplied audio file and reliably produce deterministic 480x320/10 fps video under Headless Chromium on the backend host.
- Playback software decisions have been handed to the [Playback firmware implementation specification](../playback-firmware/spec.md); its domain, loader, and Range-reader tickets now use immutable `content_id` without a separate revision field.
- Playback functional implementation may proceed without a physical speaker; pairing and final speaker acceptance remain deferred to the hardware-debugging stage.
- After functional firmware tickets are implemented, [provide the fixed prototype speaker](issues/44-provide-prototype-speaker.md), execute [the simultaneous BLE and A2DP prototype](issues/19-prototype-dual-bluetooth-coexistence.md), then execute [the MP4/H.264 and MP3 playback prototype](issues/20-prototype-h264-mp3-playback.md).
- When issue 57 resolves, collapse the backend decisions through `/to-spec`, split them through `/to-tickets`, and begin backend implementation blockers-first.
- After cloud rendering and both hardware prototypes resolve, define [end-to-end demo acceptance](issues/18-define-demo-acceptance.md).

## Not yet specified

- The exact headless capture seam, Chromium flags, deterministic frame-stepping strategy, render-time resource usage, and fallback if MediaRecorder timing is unsuitable; issue 57 must answer these.
- Exact backend route names, Pydantic shapes, SQLite migrations, Object Store interface methods, worker recovery queries, and dependency-ordered implementation tickets; these belong to the backend specification after issue 57.
- H.264 decoder implementation, reference-frame memory, bitrate ceiling, exact Playback ring-buffer sizes, and calibrated speaker latency after the media prototype.
- The exact fixed speaker name/address until a physical A2DP Sink is supplied and approved.
- Final end-to-end acceptance procedure after all preceding prototypes and implementation work resolve.

## Out of scope

- Password login, email verification, social login, account recovery, refresh tokens, organizations, and user profiles.
- Device enrollment, cloud pairing, attestation, token refresh/rotation, per-session grants, signed URLs, and hostile-network hardening.
- External Redis/message-queue infrastructure, PostgreSQL, distributed workers, or required S3-compatible storage for the competition build.
- Local STL rendering or any mandatory 3D model asset.
- User-uploaded video, Preset Video pools, and required creator-authored title/description metadata.
- Multiple user-selectable visualization themes or automatic model-generated visual styles.
- General-purpose playback of arbitrary internet media formats.
- Multi-item playback queues for the first competition demo.
- General-purpose Bluetooth speaker selection UI.
- PN532 wiring, pin selection, electrical bring-up, and low-level transport integration.
- Mobile Sharing Interface implementation before its external code is synchronized.
- Physical NFC card UID registration or cloud ownership of individual tags.
- Management, flashing, serial access, or role assignment for any additional board not explicitly assigned to this workstream.
