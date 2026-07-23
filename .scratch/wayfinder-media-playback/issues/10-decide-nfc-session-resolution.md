# Decide how NFC resolves into a Playback Session

Type: grilling
Status: resolved
Blocked by:

## Question

What exact data is stored on NFC, which board contacts the Cloud Media Service, and what session-setup payload crosses the Board Link?

## Answer

The NFC tag stores a compact HTTP URL rather than a package ID, full manifest, or media URL set. The Trigger Board reads that URL and requests the detailed content description from the Cloud Media Service.

After resolution, the Trigger Board creates a Playback Session and sends a compact, device-oriented session descriptor to the Playback Board over BLE. The descriptor includes the session identity, device-ready media references, duration, and the video/audio profile required for playback. The exact field schema is owned by the manifest-contract decision, but the Playback Board does not independently resolve the NFC URL.
