# Decide buffering and audio/video synchronization architecture

Type: grilling
Status: resolved
Blocked by: 06, 07, 14, 15

## Question

Given the selected MP4/H.264 plus MP3 device profile, how should Playback stage media and which clock controls synchronization for clips up to 30 seconds?

## Answer

Use bounded streaming with partial prebuffering rather than fully staging every asset:

- fetch the MP4 `ftyp`/`moov` metadata before playback and build bounded video sample tables;
- prebuffer approximately 0.5–1 second of decoded PCM;
- prefetch a bounded queue of compressed H.264 samples and prepare approximately 2–3 decoded display frames;
- continue reading `video.mp4` and `audio.mp3` with HTTP Range while playing;
- maintain separate compressed video, compressed audio, decoded video, and decoded PCM buffers so network jitter cannot directly starve A2DP;
- use decoded PCM samples consumed by the A2DP source path as the media clock;
- decode H.264 samples in dependency order and schedule decoded frames by MP4 presentation timestamp;
- when video is late, continue required dependency decoding but drop obsolete frame presentation rather than pausing audio;
- apply a calibrated fixed speaker-output latency offset after the prototype measures the selected sink.

Seeking selects the closest MP4 sync sample at or before the requested position, reinitializes the H.264 decoder with codec configuration, decodes forward, and suppresses presentation until the requested timestamp. MP3 seeking continues to use `audio.idx` and a short decoder preroll.

Playback starts only after minimum audio/video buffers are ready and the Fixed Speaker is connected. Buffer sizes, decoder task priority, bitrate ceiling, and display queue depth may be tuned by the hardware prototype without changing the architecture.
