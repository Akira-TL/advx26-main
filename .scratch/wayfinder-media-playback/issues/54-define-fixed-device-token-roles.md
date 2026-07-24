# Define fixed Trigger and Playback token roles

Type: grilling
Status: resolved
Blocked by: 03, 10

## Question

How does the Cloud Media Service distinguish the single competition Trigger Board from the single Playback Board without implementing device enrollment, pairing, or ticket rotation?

## Answer

The competition deployment uses two fixed opaque Bearer Tokens supplied through local server and firmware configuration:

- one Trigger Token mapped to stable identity `trigger-01` and role `TRIGGER`;
- one Playback Token mapped to stable identity `playback-01` and role `PLAYBACK`.

The tokens do not expire, rotate, refresh, or require a signing protocol during the competition build. They are not issued through a public device-registration endpoint and are not stored in SQLite as device records. The backend compares presented values in constant time against secrets loaded from environment or other Git-ignored deployment configuration.

Role permissions are intentionally narrow:

- the Trigger Token may resolve a READY Compact Content URL and retrieve Trigger-facing playback metadata;
- the Playback Token may retrieve the normalized playback descriptor and perform authenticated `GET`/`HEAD`/single-range reads for `video.mp4`, `audio.mp3`, and `audio.idx`;
- neither device token may upload source audio, claim ownership, list user content, delete content, or issue User Tokens.

The first build does not model Trigger–Playback pairing in the cloud. BLE discovery and the fixed physical device assignment are sufficient for the demo. Production device enrollment, rotation, revocation, attestation, per-session grants, and hostile-network hardening remain out of scope.
