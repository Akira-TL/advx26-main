# Prototype synchronized JPEG and MP3 playback

Type: prototype
Status: open
Blocked by: 14, 15, 16, 19

## Question

Can the Playback Board render a 30-second indexed JPEG frame stream while decoding MP3 to PCM, feeding A2DP Source, and maintaining BLE command/progress traffic within the competition targets?

Use `video.mjpg`, `video.idx`, and `audio.mp3`. Test 480x320 at 10 fps first and 15 fps as a stretch target. Record frame drops, JPEG decode time, MP3 decode load, memory high-water mark, compressed-input buffer depth, PCM underruns, controller errors, audible artifacts, and visible audio/video offset.

The prototype must use the selected partial-prebuffer architecture and PCM-derived media clock. It may tune JPEG quality, buffer sizes, and task priorities, but it must not silently replace the agreed `t5ai-jpeg-mp3-v1` media profile.
