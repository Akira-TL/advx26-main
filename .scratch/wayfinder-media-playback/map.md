# Wayfinder Map: T5AI cloud media playback

## Destination

Produce an implementation-ready technical specification, ADR set, and dependency-ordered development tickets for a competition demo where one T5AI Trigger Board reads NFC and remotely controls an identical T5AI Playback Board over BLE; the Playback Board retrieves synchronized cloud video/audio, renders video on its 3.5-inch display, and plays audio through a Bluetooth Speaker.

## Notes

- Planning artifacts use the local Markdown tracker under this directory.
- The user prefers batching genuinely independent questions in one round.
- Both managed boards use the same Tuya T5AI board and attached 3.5-inch ILI9488/GT1151 display.
- The active repository root is `/home/akira/Projects/advx26`.
- Any additional connected board remains unmanaged until the user explicitly assigns it; it must not be flashed, opened over serial, or inferred as Trigger/Playback.
- `backend/` is a Git submodule containing the FastAPI Cloud Media Service.
- Competition speed is more important than production-grade authentication or abuse resistance.
- Research must prefer primary TuyaOpen, Tuya, Beken, Bluetooth, codec, and framework documentation.
- Wayfinder produces decisions, not the final implementation. When the map clears, hand it to specification and ticket generation.

## Decisions so far

- [Define the two board roles](issues/01-define-board-roles.md) — Use identical T5AI hardware with fixed Trigger Board and Playback Board roles.
- [Separate the two Bluetooth links](issues/02-separate-bluetooth-links.md) — BLE is a bidirectional control/status link between boards; Bluetooth audio is a separate link from Playback Board to speaker.
- [Set the competition security boundary](issues/03-set-security-boundary.md) — Direct unauthenticated backend access is acceptable for the demo.
- [Make the backend the media contract owner](issues/04-backend-contract-owner.md) — The FastAPI submodule owns the evolving media contract; STL and local 3D rendering leave the product path.
- [Make the Trigger Board the playback authority](issues/05-make-trigger-board-playback-authority.md) — NFC starts a session owned by the Trigger Board, which remotely controls playback and receives acknowledgements, state, progress, completion, and errors.
- [Set the media duration and synchronization target](issues/06-set-media-duration-and-sync-target.md) — Audio and video have equal duration, are at most 30 seconds, and may differ by roughly 100–300 ms in the demo.
- [Set replacement behavior and start latency](issues/07-set-replacement-and-start-latency.md) — New content interrupts current playback; target playback start is 3–5 seconds after a valid request.
- [Use synchronous cloud media preparation for the demo](issues/08-use-synchronous-cloud-transcoding.md) — The MVP synchronously normalizes audio, pairs a Preset Video, and publishes constrained fast-start MP4/H.264 plus independent MP3 assets; future generated visuals are asynchronous.
- [Fix the Bluetooth speaker target](issues/09-fix-the-speaker-target.md) — The first demo auto-connects to one configured speaker instead of implementing a general picker.
- [Resolve NFC into a Playback Session](issues/10-decide-nfc-session-resolution.md) — NFC stores a compact URL; the Trigger Board resolves it and sends a device-oriented session descriptor over BLE.
- [Define the Board Link command and report model](issues/11-decide-board-link-command-model.md) — Support load, play, pause, seek, stop, correlated ACK/NACK, immediate state/error reports, and 500 ms progress reports.
- [Keep all interaction on the Trigger Board](issues/12-decide-playback-interaction-semantics.md) — Playback Board is output-only; the Trigger Board shows playback information, progress, controls, and interaction feedback.
- [Research simultaneous Board Link and Speaker Link capability](issues/13-research-dual-bluetooth-capability.md) — T5AI exposes BLE GATT and A2DP Source together, but sustained coexistence must pass an early hardware prototype.
- [Select the device-ready MP4/H.264 and MP3 profile](issues/14-research-media-decode-capability.md) — Use constrained `video.mp4` plus independent `audio.mp3`/`audio.idx` under `t5ai-h264-mp3-v1`; MP4 demux and H.264 decoding are explicit Playback responsibilities.
- [Define the device media manifest contract](issues/15-decide-media-manifest-contract.md) — Compact Content JSON separates Trigger presentation from a normalized `t5ai-h264-mp3-v1` playback descriptor.
- [Use a landscape Playback Board profile](issues/21-decide-playback-display-orientation.md) — Cloud output is 480x320 so the device does not rotate video frames at runtime.
- [Define the Trigger Control Panel contract](issues/22-decide-trigger-control-panel-contract.md) — Show generated identity, duration, state, progress, standard controls, and feedback without creator-authored metadata or custom actions.
- [Define automatic playback lifecycle](issues/23-decide-playback-lifecycle-behavior.md) — NFC resolution auto-loads and auto-plays; completion holds the final frame and exposes replay on Trigger.
- [Use an audio-only sharing source contract](issues/24-decide-upload-source-contract.md) — Users submit only a Shared Sound; the MVP pairs it with a cloud-owned Preset Video and produces MP4/H.264 plus independent MP3 assets.
- [Return immutable Trigger-facing JSON from Compact Content URLs](issues/25-decide-compact-url-contract.md) — Each ready revision has a direct, immutable URL and no redirect or second lookup.
- [Assign Preset Videos deterministically](issues/26-decide-preset-video-selection.md) — New packages choose from the enabled pool by stable content ID and persist the selected preset.
- [Generate neutral labels for untitled sounds](issues/27-decide-untitled-sound-label.md) — Trigger shows `声音碎片 #XXXX` plus recording/submission time when available.
- [Normalize Shared Sounds to at most 30 seconds](issues/28-decide-audio-duration-policy.md) — Keep the first 30 seconds, then loop or trim the Preset Video to exactly match.
- [Use a phone sharing and NFC publishing flow](issues/29-decide-sharing-and-nfc-publishing-flow.md) — The phone records/uploads audio, waits for readiness, and writes the immutable Compact Content URL to NFC.
- [Keep both board applications in one repository](issues/30-decide-firmware-repository-layout.md) — Use shared protocol/domain code with separate Trigger and Playback applications.
- [Treat PN532 as an external NDEF URL source](issues/31-decide-trigger-nfc-hardware.md) — This workstream consumes a URL callback and does not own PN532 wiring, pins, or transport driver integration.
- [Use compile-time demo networking](issues/32-decide-demo-network-provisioning.md) — Both boards use local, Git-ignored Wi-Fi/public-backend configuration without a provisioning UI.
- [Pair one fixed speaker once](issues/33-decide-fixed-speaker-identity.md) — Playback persists the pairing record and reconnects only to that A2DP Sink.
- [Keep the Sharing Interface external](issues/34-decide-sharing-interface-scope.md) — This repository owns the backend contract; mobile client code will be synchronized separately.
- [Discover Playback by Board Link Service UUID](issues/35-decide-board-link-discovery.md) — Trigger auto-connects at boot and continuously retries without a user-facing picker.
- [Use JSON messages over a binary BLE envelope](issues/36-decide-board-link-message-encoding.md) — One write-with-response Command characteristic and one Notify Report characteristic share the protocol.
- [Bound and fragment Board Link messages](issues/37-decide-board-link-fragmentation-limits.md) — Reassembled JSON is capped at 4 KiB, individual URLs at 1024 bytes, and incomplete assemblies expire after five seconds.
- [Serialize commands with retry-safe sequence IDs](issues/38-decide-command-retry-and-idempotency.md) — One command is in flight, ACK timeout is one second, retries are capped at three, and duplicate execution is suppressed.
- [Resynchronize sessions after reconnect](issues/39-decide-board-link-handshake-and-resync.md) — HELLO negotiates version/capabilities; GET_STATUS adopts live state or reloads a lost session at the last confirmed position.
- [Wait for the fixed speaker instead of playing silently](issues/40-decide-speaker-unavailable-playback.md) — Speaker loss pauses the media clock and resumes automatically after the configured sink reconnects.
- [Assign stable physical board identities](issues/41-assign-physical-board-identities.md) — `5AAE167197` is Playback and `5AAE167460` is Trigger; all other connected boards remain unmanaged.
- [Defer the physical speaker identity](issues/42-identify-prototype-speaker.md) — No prototype speaker is currently available, so hardware media work must not substitute another Bluetooth device.
- [Set the dual-Bluetooth prototype acceptance threshold](issues/43-decide-dual-bluetooth-prototype-acceptance.md) — Require a 60-second zero-disconnect run, 500 ms reports, 20 timely command/ACK exchanges, and no underruns or audible interruption.
- [Use partial prebuffering with a PCM-derived media clock](issues/16-decide-buffering-and-sync-architecture.md) — Parse MP4 metadata first, prebuffer decoded PCM plus bounded compressed/decoded H.264 queues, decode video in dependency order, and never stall audio for video.
- [Bound media corruption and decoder recovery](issues/17-decide-disconnect-and-recovery.md) — Recover H.264 at the next valid IDR, resynchronize isolated MP3 failures with timeline-preserving silence, and distinguish retryable network failure from immutable content corruption.
- [Use MP4 sample tables plus an indexed MP3 asset](issues/45-decide-media-index-and-integrity.md) — Video timing, ranges, sync samples, and codec configuration come from constrained MP4 metadata; only MP3 retains a fixed-width binary index.
- [Use bounded in-memory pause, seek, and replay caching](issues/46-decide-pause-seek-cache-behavior.md) — Pause prefetches only to configured high-water marks; out-of-buffer seek resets decoder queues and reopens indexed Range reads without changing the session identity.
- [Publish Media Packages atomically](issues/47-decide-atomic-media-publication.md) — Build and validate a private staging revision, then expose the immutable READY descriptor and every asset together.
- [Freeze Board Link UUIDs and the v1 fragment header](issues/48-decide-board-link-wire-constants.md) — Use stable private Service/Command/Report UUIDs and a fixed 16-byte little-endian envelope without an additional application checksum.
- [Normalize common phone audio into bounded MP3](issues/49-decide-source-audio-normalization.md) — Inspect source bytes with FFprobe, accept common phone audio formats up to 50 MiB, retain at most 30 seconds, and publish 44.1 kHz 128 kbps CBR MP3 with at most two channels.
- [Use immutable byte-range HTTP transport](issues/50-decide-immutable-http-transport.md) — Published revision resources expose strong SHA-256 ETags, year-long immutable caching, HEAD, single byte ranges, `If-Range`, exact lengths, and strict validator checks.
- [Freeze Playback states and errors](issues/51-decide-playback-state-and-errors.md) — Use nine stable states, eight stable fatal error categories, immediate state reports, retained play/pause intent, and enum-driven Trigger behavior.
- [Freeze the Board Link JSON body schema](issues/52-decide-board-link-json-schema.md) — Every body uses one strict versioned envelope, fixed command/report enums, bounded required fields, media positions instead of wall-clock timestamps, and stable ACK/NACK correlation.

## Active decision frontier

- Playback software decisions are complete and have been handed to the [Playback firmware implementation specification](../playback-firmware/spec.md).
- Playback functional implementation may proceed without a physical speaker; pairing and speaker identity are deferred to the hardware-debugging stage.
- After the functional tickets are implemented, [provide the fixed prototype speaker](issues/44-provide-prototype-speaker.md), execute [the simultaneous BLE and A2DP prototype](issues/19-prototype-dual-bluetooth-coexistence.md), then execute [the MP4/H.264 and MP3 playback prototype](issues/20-prototype-h264-mp3-playback.md).
- After both hardware prototypes resolve, define [end-to-end demo acceptance](issues/18-define-demo-acceptance.md).

## Not yet specified

- H.264 decoder implementation, reference-frame memory, bitrate ceiling, exact ring-buffer sizes, and calibrated speaker latency after the media prototype.
- Backend schema migration, audio normalization implementation, index generation, and atomic publication ticket breakdown.
- Firmware module boundaries and test seams after the hardware prototypes settle the runtime seams.
- The exact fixed speaker name/address until a physical A2DP Sink is supplied and approved.
- Final end-to-end acceptance procedure after all preceding decisions resolve.

## Out of scope

- Production authentication, authorization, signed URLs, and hostile-network hardening.
- Local STL rendering or any mandatory 3D model asset.
- User-uploaded video and required creator-authored title/description metadata.
- Automatic generated visual effects in the first competition build.
- General-purpose playback of arbitrary internet media formats.
- Multi-item playback queues for the first competition demo.
- General-purpose Bluetooth speaker selection UI.
- PN532 wiring, pin selection, electrical bring-up, and low-level transport integration.
- Mobile Sharing Interface implementation before its external code is synchronized.
- Management, flashing, serial access, or role assignment for any additional board not explicitly assigned to this workstream.
