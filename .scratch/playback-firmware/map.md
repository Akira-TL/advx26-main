# Playback firmware implementation

## Decisions so far

- [Define Playback domain and state model](issues/01-define-playback-domain.md) — Canonical commands, reports, states, errors, sessions, snapshots, and transition helpers are implemented.
- [Implement Board Link body and fragment codec](issues/02-implement-board-link-codec.md) — Fixed binary framing, bounded reassembly, strict JSON commands, report serialization, and stable NACK mapping are implemented.
- [Implement Media Package descriptor and audio index loader](issues/04-implement-media-package-loader.md) — The constrained H.264/MP3 profile, immutable assets, and fixed-width MP3 index are validated before decode.
- [Implement immutable HTTP range reader](issues/05-implement-http-range-reader.md) — HEAD/Range identity checks, bounded reads, cancellation, and stable transport errors are implemented.
- [Implement constrained MP4 demux and sample source](issues/06-implement-mp4-demux.md) — Fast-start MP4 sample tables, sync lookup, Range-backed access units, and Annex-B conversion are implemented.
- [Implement Board Link GATT peripheral](issues/03-implement-board-link-gatt.md) — Fixed peripheral advertising, worker-thread command delivery, HELLO negotiation, ATT-aware report fragmentation and reconnectable transport state are implemented.
- [Implement H.264 decoder and video output](issues/13-implement-h264-video-output.md) — Baseline H.264 decode, IDR reset, YUV420P-to-RGB565 conversion, stale-frame suppression, and LVGL presentation are implemented.
- [Implement indexed MP3 audio path](issues/07-implement-mp3-audio-path.md) — Indexed immutable reads, strict Helix decoding, bounded PCM buffering, exact sample accounting, seek warm-up, pause controls, and silence recovery are implemented.
- [Implement A2DP Speaker Link functionality](issues/08-implement-speaker-link.md) — Fixed-target A2DP Source state, exact PCM consumption, SBC negotiation/encoding, bounded reconnect, dual-host configuration, and reproducible T5AI partitioning are implemented.
- [Implement PCM media clock and MP4/H.264 scheduler](issues/09-implement-media-scheduler.md) — PCM-authoritative timing, bounded audio/video queues, late-frame suppression, seek, pause, replay, completion, and final-frame hold are implemented.
- [Integrate Playback engine commands, state, and reports](issues/10-integrate-playback-engine.md) — Serialized command policy, idempotent sequence handling, immediate ACK/NACK, session replacement, progress, completion, errors, and authoritative snapshots are implemented.
- [Implement Playback recovery and resynchronization](issues/11-implement-recovery.md) — Immutable-resource failures, MP3 silence recovery, bounded IDR recovery, speaker-loss freezing, reconnect intent, and stale-generation rejection are implemented.
- [Integrate Playback application bootstrap](issues/12-integrate-playback-app.md) — Board Link-driven application composition, output-only screens, fixed Wi-Fi bootstrap, engine/report wiring, real BLE naming, and project-owned Helix linkage are implemented.

## Active implementation frontier

- All planned Playback firmware implementation tickets are resolved.
- Remaining work is hardware-stage configuration and acceptance: fixed speaker address/pairing, flash and serial verification, audible A2DP output, BLE/Wi-Fi coexistence, and final speaker-latency tuning.
