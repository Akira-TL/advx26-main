# Decide playback behavior when the fixed speaker is unavailable

Type: grilling
Status: resolved
Blocked by: 09, 23, 33

## Question

When a valid session is ready but Playback Board cannot connect to the fixed Bluetooth Speaker, should video wait, play silently, or fail the session, and what should Trigger show while automatic speaker reconnection continues?

## Answer

Do not advance silent video while the fixed speaker is unavailable.

Before playback starts, Playback Board enters `WAITING_SPEAKER`, reports that state over the Board Link, keeps the prepared session, and continuously retries the stored speaker target. Trigger Board shows a concise user-facing `音箱未连接` state while retaining the normal controls and content identity.

If the Speaker Link drops during playback, Playback Board pauses the media clock and holds the current video frame. It reports `WAITING_SPEAKER` with the current position, reconnects only to the configured speaker, and automatically resumes from that position after the Speaker Link is usable again when the last Trigger-authoritative state was playing. A session that was paused remains paused.

The MVP does not play video silently, fall back to another discovered speaker, or route audio to an unspecified board output. Explicit `STOP`, a replacement `LOAD_SESSION`, or an unrecoverable media error still takes precedence over speaker reconnection.
