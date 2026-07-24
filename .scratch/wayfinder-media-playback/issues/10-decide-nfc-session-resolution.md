# Decide how NFC resolves into a Playback Session

Type: grilling
Status: resolved
Blocked by: 03, 54

## Question

What exact data is stored on NFC, which board contacts the Cloud Media Service, and what authenticated session-setup payload crosses the Board Link?

## Answer

The NFC tag stores only the immutable Compact Content URL for one READY content item. It does not store a User Token, Trigger Token, Playback Token, complete manifest, media byte URLs, ownership data, or a physical-card registration identity.

The Trigger Board reads the URL and resolves it with its fixed Trigger Bearer Token. The Cloud Media Service verifies role `TRIGGER` and returns the Trigger-facing content description. The URL itself is therefore shareable but is not sufficient for anonymous cloud access.

After resolution, the Trigger Board creates a local Playback Session and sends the normalized playback descriptor, `content_id`, duration, and local `session_id` to Playback over BLE. It never sends its Trigger Token or a user's token.

Playback uses its own fixed Playback Bearer Token when retrieving the playback descriptor or immutable media objects referenced by that session. The cloud does not pair the two boards, issue a per-session grant, or record the local playback session in SQLite for the competition build.
