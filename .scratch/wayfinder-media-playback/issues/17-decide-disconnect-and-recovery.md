# Decide disconnect and recovery behavior

Type: grilling
Status: resolved
Blocked by: 11, 38, 39, 40

## Question

Existing decisions already define Board Link reconnect/resynchronization, command retry/idempotency, Speaker Link waiting/resume, and HTTP Range retry exhaustion. The remaining recovery policy is MP4 corruption and H.264/MP3 decoder failure.

## Answer

Use bounded recovery that preserves the audio timeline without hiding persistent corruption:

- MP4 box, sample-table, codec-configuration, sample-range, or track-layout errors fail the session as immutable content corruption. Playback does not guess missing sample metadata.
- An isolated H.264 sample decode failure suppresses video presentation and begins recovery at the next valid sync sample/IDR. The decoder is reset and reconfigured from MP4 codec initialization data before decoding resumes.
- H.264 recovery continues to decode required samples in order; it must not present predicted frames whose reference chain is invalid.
- Failure to find and decode a valid IDR within approximately two seconds of media time, or two consecutive GOP recovery failures, terminates the session with `H264_DECODE_FAILED`.
- MP3 decoding searches for the next valid indexed frame after an isolated decode error. The missing interval is represented by equal-duration silence so the PCM-derived media clock remains continuous. Three consecutive MP3 frame failures, or failure to resynchronize within 500 ms of media time, terminates the session.
- Network retry exhaustion is reported to Trigger as a retryable network error.
- Immutable asset length, MP4 metadata, audio index, range, ETag, or checksum mismatch is reported as `CONTENT_INVALID`. Trigger shows “内容暂不可用” and does not automatically loop requests against the same immutable content identity.

Decoder recovery never rewinds the current session silently, never blocks audio while waiting for video recovery, and never substitutes another visual or content item for corrupt immutable media.
