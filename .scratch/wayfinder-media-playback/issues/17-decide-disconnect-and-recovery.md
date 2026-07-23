# Decide disconnect and recovery behavior

Type: grilling
Status: resolved
Blocked by: 11, 38, 39, 40

## Question

Existing decisions already define Board Link reconnect/resynchronization, command retry/idempotency, Speaker Link waiting/resume, and HTTP Range retry exhaustion. The remaining recovery policy is media corruption and decoder failure:

1. How many individual JPEG frame decode failures may be skipped before the session fails?
2. Should MP3 decoding attempt frame resynchronization, and what threshold turns the error into a fatal session failure?
3. What should Trigger show for an immutable asset whose length, index, or checksum is invalid: a normal retry action, a distinct content-unavailable state, or both?

## Answer

Use bounded recovery that preserves the audio timeline without hiding persistent corruption:

- A single JPEG decode failure is skipped while audio continues. Three consecutive JPEG frame failures, an invalid frame range, or an index entry outside the immutable stream terminates the session as a media error.
- MP3 decoding searches for the next valid frame header after an isolated decode error. The missing interval is represented by equal-duration silence so the PCM-derived media clock remains continuous. Three consecutive MP3 frame failures, or failure to resynchronize within 500 ms of media time, terminates the session.
- Network retry exhaustion is reported to Trigger as `NETWORK_ERROR` with a user-visible retry action.
- Immutable asset length, index, range, or checksum mismatch is reported as `CONTENT_UNAVAILABLE`. Trigger shows “内容暂不可用” and does not automatically loop requests against the same revision. A new revision or explicit later user retry may attempt it again.

Decoder recovery never rewinds the current session silently, never blocks audio while waiting for video, and never substitutes another Preset Video for a corrupt immutable revision.
