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

## Active implementation frontier

- [Implement PCM media clock and MP4/H.264 scheduler](issues/09-implement-media-scheduler.md) is claimed.
- Engine, recovery, and application bootstrap remain blocked by their listed dependencies.
