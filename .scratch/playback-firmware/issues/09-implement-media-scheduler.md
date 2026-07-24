# Implement PCM media clock and MP4/H.264 scheduler

Type: task
Status: claimed
Blocked by: 06, 07, 08, 13

## Goal

Coordinate bounded audio/video buffering and schedule decoded H.264 frames from MP4 timestamps while actual PCM consumption remains authoritative.

## Functional scope

- prepare parsed MP4 metadata and the MP3 audio index before playback;
- prebuffer approximately 0.5–1 second of decoded PCM;
- maintain bounded compressed H.264 sample queues and approximately 2–3 decoded display frames before entering `PLAYING`;
- derive `position_ms` from PCM samples actually consumed by the Speaker Link;
- request H.264 samples in decode dependency order and associate decoded frames with MP4 PTS;
- present the decoded frame corresponding to the audio position plus a configurable fixed speaker-latency offset;
- when video is late, continue required dependency decoding but suppress obsolete frame presentation instead of stalling PCM;
- freeze the media clock while paused, seeking, buffering, or waiting for the fixed speaker;
- continue bounded prefetch while paused until configured high-water marks are reached;
- support seek by cancelling stale reads, selecting the sync sample at or before the target, resetting H.264/MP3 decoders, reapplying SPS/PPS, warming MP3 decode, decoding video forward, and suppressing pre-target output;
- preserve play/pause intent across buffering and seek;
- signal natural completion only after audio duration is consumed and the final decoded video frame is held.

## Module seam

Expose prepare, play, pause, seek, stop, and snapshot operations. The scheduler coordinates the video source, MP3 decoder, and Speaker Link but does not expose their buffer structures or codec handles.

## Completion criteria

- audio progress is never inferred from wall-clock time alone;
- video presentation cannot block or rewind the audio clock;
- arbitrary H.264 dependency samples are not skipped merely because their presentation timestamp is late;
- pause, seek, replay, speaker loss, IDR recovery, and final-frame hold share one media-position model.

## Out of scope

Board Link serialization, tests, build, flash, physical speaker pairing, and final latency tuning.
