# Decide buffering and audio/video synchronization architecture

Type: grilling
Status: resolved
Blocked by: 06, 07, 14, 15

## Question

Given the selected JPEG/MP3 device profile, how should Playback stage media and which clock controls synchronization for clips up to 30 seconds?

## Answer

Use bounded streaming with partial prebuffering rather than fully staging every asset:

- fetch `video.idx` before playback;
- prebuffer approximately 0.5–1 second of decoded PCM;
- prefetch approximately 3–5 indexed JPEG frames;
- continue reading `video.mjpg` and `audio.mp3` with HTTP Range while playing;
- maintain separate compressed-input and decoded-PCM buffers so network jitter cannot directly starve A2DP;
- use decoded PCM samples consumed by the A2DP source path as the media clock;
- schedule JPEG frames by presentation timestamp;
- wait for an early frame, but drop a late frame instead of pausing audio;
- apply a calibrated fixed speaker-output latency offset after the prototype measures the selected sink.

Playback starts only after both minimum buffers are ready and the Fixed Speaker is connected. Buffer sizes and task priorities may be tuned by the hardware prototype without changing the architecture.
