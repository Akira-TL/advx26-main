# Device-ready media capability research

## Updated conclusion

The selected product contract is now `t5ai-h264-mp3-v1`: constrained fast-start MP4/H.264 video plus independent MP3 audio and `audio.idx`.

This supersedes the earlier indexed JPEG recommendation. The earlier source inspection remains relevant only as a risk statement: TuyaOpen v1.9.0 did not expose an obvious ready-made cloud-file MP4 demux plus H.264 decode path. Therefore Playback must explicitly implement or integrate those capabilities instead of assuming the SDK already provides them.

## Confirmed platform facts

- T5-E1-IPEX material documents H.264 encoding capability; it does not by itself establish a complete local MP4/H.264 playback stack.
- TuyaOpen exposes JPEG decode/display paths and MP3 decode support.
- The public A2DP Source interface consumes PCM and supports the sample-rate/channel formats needed by the independent MP3 path.
- No reusable general MP4 demux plus H.264 cloud-file decoder was identified in the inspected TuyaOpen v1.9.0 application path.

## Selected bounded profile

### Video

- Asset: `video.mp4`.
- Container: non-fragmented ISO BMFF/MP4 with `moov` before `mdat`.
- Tracks: exactly one H.264 video track; no audio, subtitle, or alternate video tracks.
- Codec: H.264 Baseline Profile, YUV420P, progressive, no B frames.
- GOP: closed and bounded; IDR interval no longer than approximately one second.
- Geometry: 480x320 landscape.
- Frame rate: 10 fps initially; 15 fps only after capability validation.
- Duration: no more than 30 seconds.
- Timestamps: monotonic, with a simple decode/presentation timeline suitable for bounded MCU playback.

### Audio

- Asset: `audio.mp3`.
- Index: `audio.idx` containing MP3 frame byte ranges and decoded PCM sample positions.
- Encoding: 128 kbps CBR, 44.1 kHz, at most two channels.
- Decode output: signed 16-bit PCM supplied to A2DP Source.

## Playback implication

Playback needs two explicit video modules:

1. an MP4 demux module that parses the constrained box/sample-table subset, reads `avcC`, locates sync samples, and maps timestamps to immutable byte ranges;
2. an H.264 decoder adapter that accepts decoder configuration and ordered samples, produces displayable frames, resets on seek/recovery, and exposes decode errors without leaking implementation details.

Audio consumption remains the media clock. H.264 samples must be decoded in dependency order. A late decoded frame may be dropped from presentation, but required reference samples cannot be skipped arbitrarily.

## Required later capability gate

After functional implementation, run a 30-second device test with MP4 Range streaming, MP4 demux, H.264 decode, MP3 decode, A2DP output, BLE control, and progress reports active together. Record video throughput, decoder/reference memory, frame drops, MP3 load, PCM underruns, Bluetooth errors, total memory high-water mark, seek-to-IDR latency, and visible synchronization error.
