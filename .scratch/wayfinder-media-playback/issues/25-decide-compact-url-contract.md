# Decide the Compact Content URL contract

Type: grilling
Status: resolved
Blocked by: 10, 15, 54

## Question

What does the NFC URL identify, how is it authenticated, and may its response change after publication?

## Answer

The NFC tag stores one stable Compact Content URL containing the immutable `content_id`. The URL directly returns the complete Trigger-facing JSON document with HTTP 200 only when the request includes the fixed Trigger Bearer Token and the content is `READY`.

The URL is intentionally safe to copy between NFC cards: knowing it does not grant anonymous media access, expose the owning User Token, or authorize content management. The backend does not record physical NFC card UIDs or create a new content record for each written card.

One upload produces one immutable READY content item and one Compact Content URL. The backend never replaces the media or descriptor bytes behind that READY identity. Regeneration creates a new `content_id` and URL.

Before READY, owner-authenticated status APIs may report processing state, but the Compact Content URL does not expose a partial descriptor. After READY, its response contains Trigger presentation data and the normalized Playback descriptor; the referenced media endpoints separately require the fixed Playback Token.
