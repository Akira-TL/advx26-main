# Implement indexed MP3 audio path

Type: task
Status: resolved
Blocked by: 04, 05

## Goal

Stream independent `audio.mp3`, decode it to bounded PCM, and expose exact consumed sample positions for synchronization and A2DP output.

## Functional scope

- locate complete MP3 frames through `audio.idx` rather than CBR byte estimates alone;
- retrieve bounded contiguous MP3 ranges through `http_range_reader`;
- validate indexed frame byte length and CRC32 before decode;
- initialize the existing TuyaOpen MP3 decoder for the package channel count and sample rate;
- decode to signed 16-bit PCM and write into a bounded ring buffer with low/high water marks;
- expose consumed/available PCM sample counts without leaking decoder internals;
- support decoder reset and a short preceding-frame warm-up window for seek;
- on an isolated MP3 failure, search the next indexed frame boundary and insert equivalent-duration silence;
- terminate after three consecutive failures or inability to recover within 500 ms.

## Module seam

Expose prepare, fill, consume, seek, pause, resume, and close operations over the independent MP3 descriptor and audio index. Hide MP3 frame parsing, decoder handles, compressed buffers, and PCM ring implementation.

## Completion criteria

- PCM output remains timeline-continuous across recoverable MP3 frame loss;
- callers can derive media position from consumed PCM samples;
- network access and MP3 decode can be paused independently while preserving bounded buffers;
- the module has no dependency on MP4 demux or H.264 decoder internals.

## Result

Implemented an indexed MP3 source with immutable HTTP Range reads, per-record CRC32 validation, strict MPEG-1 Layer III/44.1 kHz/128 kbps format checks, Helix decode into a bounded PSRAM PCM ring, exact consumed-frame accounting, independent fetch/output pause controls, ten-record seek warm-up, timeline-preserving silence recovery, and stable fatal termination after three consecutive failures or a 500 ms recovery deadline.

Validation completed with strict host compilation, allocator ownership checks, normal/pause/seek/recovery tests, AddressSanitizer/UndefinedBehaviorSanitizer, GCC static analysis, Helix symbol resolution, and a full `TUYA_T5AI_BOARD` firmware build.

## Out of scope

A2DP connection behavior, MP4/H.264 scheduling, flash, and audible-quality validation.
