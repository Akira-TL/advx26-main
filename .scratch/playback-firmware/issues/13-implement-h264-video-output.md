# Implement H.264 decoder and video output

Type: task
Status: resolved
Blocked by: 06

## Goal

Integrate or implement the actual H.264 decoder behind a small adapter, decode ordered access units from the MP4 sample source, and present timestamped 480x320 frames on the LCD.

## Functional scope

- define a decoder interface with open/configure, submit access unit, receive decoded frame, reset, and close operations;
- select and integrate the concrete H.264 decoder implementation rather than assuming TuyaOpen already provides a usable cloud-file decoder;
- apply SPS/PPS and NAL framing from the MP4 demux module at startup, seek, and recovery;
- accept only Baseline Profile, YUV420P, progressive, no-B-frame streams within the selected geometry/frame-rate bounds;
- decode H.264 access units in dependency order;
- expose decoded frames with PTS, dimensions, pixel format, stride, and explicit ownership/lifetime;
- convert or adapt decoder output to the LCD-compatible frame format behind the video-output interface;
- retain sufficient display buffers so the currently shown frame remains valid while the next frame is decoded;
- allow the scheduler to suppress obsolete presentation without skipping required dependency decode;
- reset reference state at a selected IDR and decode forward for seek/recovery;
- preserve the final successfully displayed frame at natural completion;
- map unrecoverable decode/configuration errors to `H264_DECODE_FAILED`.

## Module seam

Expose configure, decode, poll frame, reset-at-sync, present, and close operations. Hide decoder handles, reference-frame storage, SPS/PPS representation, YUV/RGB conversion buffers, and LVGL locking.

## Completion criteria

- a validated MP4 access unit can be decoded and associated with its original PTS;
- decoder reset at an IDR produces a valid new reference chain;
- invalid predicted output is never presented after corruption or seek;
- the display path contains no JPEG decoder dependency;
- decoder implementation can later be replaced without changing MP4 demux, scheduler, or Playback engine interfaces.

## Out of scope

MP4 box parsing, HTTP transport, MP3/A2DP, tests, build, flash, final performance tuning, and hardware acceptance.

## Answer

Integrated a pinned Baseline H.264 software decoder with PSRAM allocation, IDR reset and frame-lifetime adapters; added YUV420P-to-RGB565 conversion with stale-frame suppression and double buffering; and connected a lock-hidden LVGL sink that retains the final successful frame. The implementation spans playback commits `449bd63`, `95a2515`, `edfdae7`, `9b38954`, `388a3d4`, and `43c98f7`.
