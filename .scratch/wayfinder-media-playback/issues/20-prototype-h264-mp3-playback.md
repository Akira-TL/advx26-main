# Prototype synchronized MP4/H.264 and MP3 playback

Type: prototype
Status: open
Blocked by: 14, 15, 16, 19

## Question

Can the Playback Board stream and decode a 30-second constrained MP4/H.264 video while decoding independent MP3 to PCM, feeding A2DP Source, and maintaining BLE command/progress traffic within the competition targets?

Use `video.mp4`, `audio.mp3`, and `audio.idx`. Test 480x320 at 10 fps first and 15 fps only after the initial profile is stable. Record:

- MP4 metadata parse time and memory;
- H.264 sample throughput, decoder load, reference-frame memory, and decoded-frame drops;
- MP3 decoder load, PCM buffer depth, and underruns;
- total memory high-water mark;
- controller errors and BLE/A2DP continuity;
- audible artifacts and visible audio/video offset;
- seek-to-IDR recovery latency.

The prototype must use the selected partial-prebuffer architecture and PCM-derived media clock. It may tune bitrate ceiling, GOP length, buffer sizes, and task priorities, but it must not silently replace the agreed `t5ai-h264-mp3-v1` media profile.
