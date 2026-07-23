# Playback Firmware Implementation Specification

## Objective

Implement the SoundPola Playback Board as an output-only executor on the Tuya T5AI board. Playback receives versioned Board Link commands over BLE, retrieves an immutable `t5ai-h264-mp3-v1` Media Package over Wi-Fi, demultiplexes and decodes constrained MP4/H.264 video, decodes independent MP3 into PCM, forwards PCM through A2DP Source, renders video on the 480x320 LCD, and reports authoritative state, progress, completion, and errors to Trigger.

## Current starting point

The `firmware/playback` submodule currently has uncommitted prototype work that:

- joins fixed demo Wi-Fi;
- downloads a temporary JSON JPEG index and individual JPEG frame URLs;
- decodes JPEG into RGB565;
- presents frames through `mob_screen_show_rgb565_frame()`;
- schedules video against local elapsed time.

This prototype is implementation input, not a completed ticket and not the final media contract. Preserve reusable HTTP download, PSRAM allocation, RGB565/LCD presentation, and screen-locking work where it remains useful. Remove the temporary JSON index, per-frame URL model, JPEG decoder path, fixed demo URL, and `t5ai-indexed-jpeg-v1` assumptions from the final functional path.

## Selected media profile

```text
t5ai-h264-mp3-v1
├── video.mp4
│   └── one constrained H.264 video track
├── audio.mp3
└── audio.idx
```

`video.mp4` requirements:

- non-fragmented fast-start MP4 with `moov` before `mdat`;
- exactly one H.264 video track and no audio/subtitle/alternate tracks;
- H.264 Baseline Profile, YUV420P, progressive, no B frames;
- closed GOP with IDR spacing no longer than approximately one second;
- 480x320 landscape;
- 10 fps initial profile, 15 fps only as a later capability target;
- duration no longer than 30 seconds.

Audio remains independent MP3 because A2DP consumes decoded PCM and the product does not need MP4 audio/AAC demux in the first implementation.

## Scope

Playback functional implementation includes:

- Playback domain types, states, errors, session identity, and snapshots;
- Board Link binary framing, JSON bodies, GATT peripheral behavior, command intake, and report publication;
- immutable HTTP `HEAD` and single-range reads with strong resource identity;
- constrained MP4 metadata parsing and sample-table lookup;
- H.264 codec-configuration handling, ordered sample decode, IDR seek/recovery, and LCD presentation;
- indexed MP3 retrieval, MP3 decode, PCM buffering, and bounded decoder recovery;
- A2DP Source output and fixed-speaker connection state handling;
- PCM-derived media clock, MP4-PTS video scheduling, pause, seek, replay, and replacement;
- application bootstrap and removal of fixed demo autoplay behavior.

## Explicitly deferred

The current planning slice does not include:

- test source changes or new test tickets;
- build, flash, serial-log, or hardware acceptance work;
- physical speaker discovery, pairing, advertised-name capture, or address capture;
- Trigger firmware, PN532, NFC, backend, or Android implementation;
- final H.264 bitrate tuning, exact buffer sizes, Bluetooth latency offset, or task priorities from hardware measurements.

## Module design

### `playback_engine`

External interface:

```c
OPERATE_RET playback_engine_init(const playback_engine_config_t *config);
OPERATE_RET playback_engine_submit(const playback_command_t *command);
OPERATE_RET playback_engine_get_snapshot(playback_snapshot_t *snapshot);
```

It owns command serialization, session replacement, state transitions, ACK/NACK semantics, progress publication, and recovery coordination. Callers do not directly operate HTTP, MP4, decoders, display, or A2DP.

### `board_link`

Owns the fixed GATT service, fragment envelope, JSON body codec, reassembly, HELLO negotiation, and Report notifications. Bluetooth callbacks enqueue complete commands; they never mutate Playback state directly.

### `media_pipeline`

Internal interface accepts prepare, play, pause, seek, stop, and snapshot requests. It hides MP4 metadata, H.264 reference state, compressed queues, MP3 decoder state, PCM rings, HTTP ranges, and display buffers.

### Internal adapters

- `http_range_reader`: immutable resource identity and byte-range retrieval;
- `mp4_demux`: constrained MP4 boxes, sample tables, timestamps, sync samples, and `avcC`;
- `h264_decoder`: decoder configuration, ordered access units, reset/recovery, and decoded-frame output;
- `video_output`: conversion/presentation of decoder output to the 480x320 display;
- `audio_decoder`: indexed MP3 to PCM;
- `speaker_link`: A2DP Source connection and PCM consumption;
- `playback_screen`: idle, waiting, error, and final-frame presentation.

## Functional invariants

- Playback never treats BLE disconnect as a command to stop media.
- A newer accepted `LOAD_SESSION` replaces the current session immediately.
- Audio consumption is the media clock; video presentation may drop late decoded frames but required H.264 dependency samples remain decoded in order.
- No media time advances while the configured speaker is unavailable.
- Published resource length, ETag/SHA-256 identity, MP4 sample bounds, codec configuration, audio-index version, offsets, lengths, and CRC32 must be validated before use.
- Seek selects an MP4 sync sample at or before the target, resets the decoder, reapplies SPS/PPS, and decodes forward without presenting pre-target frames.
- `STOP` returns to idle and releases the active media pipeline.
- Natural completion holds the final decoded video frame until replay, replacement, or stop.
- H.264 recovery begins at a valid IDR; invalid reference chains are never presented.
- Trigger behavior depends on stable state/error enums, never free-form log text.

## Implementation dependency order

```text
01 Playback domain
├── 02 Board Link codec → 03 GATT peripheral
├── 04 Media Package descriptor
└── 05 HTTP Range reader
    ├── 06 MP4 demux → 13 H.264 decoder/video output ┐
    └── 07 MP3 decode → 08 A2DP Speaker Link         ├→ 09 Scheduler
                                                       └→ 10 Engine → 11 Recovery → 12 App integration
```

Tickets 02, 04, and 05 may proceed in parallel after ticket 01. The highest-risk media chain is 05 → 06 → 13 and should be implemented before treating the remaining Playback integration as low risk.

## Tickets

- [01 — Define Playback domain and state model](issues/01-define-playback-domain.md)
- [02 — Implement Board Link codec](issues/02-implement-board-link-codec.md)
- [03 — Implement Board Link GATT peripheral](issues/03-implement-board-link-gatt.md)
- [04 — Implement Media Package loader](issues/04-implement-media-package-loader.md)
- [05 — Implement immutable HTTP range reader](issues/05-implement-http-range-reader.md)
- [06 — Implement constrained MP4 demux and sample source](issues/06-implement-mp4-demux.md)
- [07 — Implement indexed MP3 audio path](issues/07-implement-mp3-audio-path.md)
- [08 — Implement A2DP speaker link](issues/08-implement-speaker-link.md)
- [09 — Implement media clock and scheduler](issues/09-implement-media-scheduler.md)
- [10 — Integrate Playback engine commands and reports](issues/10-integrate-playback-engine.md)
- [11 — Implement recovery and resynchronization](issues/11-implement-recovery.md)
- [12 — Integrate Playback application bootstrap](issues/12-integrate-playback-app.md)
- [13 — Implement H.264 decoder and video output](issues/13-implement-h264-video-output.md)
