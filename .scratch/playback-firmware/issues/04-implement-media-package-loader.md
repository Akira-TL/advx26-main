# Implement Media Package descriptor and audio index loader

Type: task
Status: resolved
Blocked by: 01

## Goal

Validate one normalized `t5ai-h264-mp3-v1` playback descriptor and load the fixed-width MP3 audio index into bounded Playback-owned structures.

## Functional scope

- validate profile name, duration, revision, dimensions, frame rate, keyframe interval, channel count, sample rate, bitrate, URLs, file lengths, SHA-256 values, and ETags;
- require `video.mp4` to declare `MP4_H264`, Baseline Profile, YUV420P, 480x320, supported frame rate, and bounded IDR interval;
- reject any descriptor that declares a video index or an MP4 audio track requirement;
- load and parse the 16-byte `audio.idx` header;
- parse 16-byte MP3 index records using little-endian reads without unaligned struct casts;
- enforce monotonic PCM sample positions, byte bounds, non-overlap, positive lengths, record-size agreement, and final indexed duration;
- provide indexed MP3 lookup by target PCM sample position;
- expose immutable MP4, MP3, and audio-index descriptors to the media pipeline;
- map invalid metadata to `CONTENT_INVALID` or `INDEX_INVALID` before any decoder starts.

## Module seam

Provide a small immutable session-descriptor interface consumed by MP4 demux, audio decode, and the Playback engine. MP4 box/sample parsing remains behind the video module rather than this descriptor loader.

## Completion criteria

- no media Range read is scheduled from an unchecked resource descriptor;
- MP3 seek can locate a safe indexed frame and preroll window;
- profile validation rejects the superseded JPEG/MJPEG contract;
- audio-index memory ownership and release belong to this module rather than the MP3 decoder.

## Out of scope

HTTP transport implementation, MP4 box parsing, H.264/MP3 decoding, tests, build, flash, and backend asset generation.

## Answer

Implemented strict `t5ai-h264-mp3-v1` session validation, immutable asset checks, H.264/MP3 profile bounds, fixed-width audio-index parsing, ownership, and PCM-position lookup in playback commit `60d8625`.
