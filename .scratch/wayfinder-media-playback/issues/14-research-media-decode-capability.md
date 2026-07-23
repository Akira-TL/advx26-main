# Research device-ready video and audio formats

Type: research
Status: resolved
Blocked by:

## Question

Which video and audio representation should the Cloud Media Service prepare for Playback Board decoding and Bluetooth speaker output?

## Answer

The selected product contract is `t5ai-h264-mp3-v1`:

- video is one `video.mp4` file with exactly one constrained H.264 video track;
- MP4 uses fast-start layout with `moov` before `mdat` so metadata can be obtained before media ranges;
- H.264 uses Baseline Profile, YUV 4:2:0, progressive frames, no B frames, closed GOP, and an IDR interval no longer than approximately one second;
- output is 480x320 landscape at 10 fps initially, with 15 fps only as a later capability target;
- audio remains a separate `audio.mp3` asset using 128 kbps CBR, 44.1 kHz, and at most two channels;
- `audio.idx` records MP3 frame boundaries and decoded sample positions for accurate seek;
- Playback decodes MP3 to PCM S16LE and supplies PCM to A2DP Source;
- decoded PCM consumption remains the synchronization clock.

Earlier JPEG/MJPEG decisions are superseded. The official T5-E1-IPEX material and inspected TuyaOpen v1.9.0 paths did not establish a ready-to-use cloud-file MP4 demux plus H.264 decode pipeline. That fact remains an implementation risk, but it no longer changes the selected media contract. Playback must explicitly implement or integrate an MP4 demuxer and an H.264 decoder adapter.

The initial H.264 bitrate ceiling, exact decoder implementation, reference-frame memory, and sustained 10/15 fps capability remain implementation and later hardware-prototype questions.

Research asset: [Device-ready media capability research](../research/14-media-decode-capability.md).
