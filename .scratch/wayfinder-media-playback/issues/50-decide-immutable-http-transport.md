# Decide authenticated immutable HTTP transport

Type: grilling
Status: resolved
Blocked by: 15, 17, 45, 47, 54

## Question

What authenticated HTTP behavior must the Cloud Media Service provide for immutable media assets and Compact Content JSON?

## Answer

Every READY content item is immutable. The Compact Content JSON requires the fixed Trigger Bearer Token. The normalized playback descriptor and `video.mp4`, `audio.mp3`, and `audio.idx` endpoints require the fixed Playback Bearer Token. Missing, unknown, or wrong-role credentials return an authentication or authorization error before object bytes are read.

Each immutable authenticated response includes representation metadata equivalent to:

```http
Accept-Ranges: bytes
ETag: "<lowercase-response-sha256>"
Cache-Control: private, max-age=31536000, immutable
Vary: Authorization
Content-Length: <exact-byte-length>
```

The ETag is a strong validator derived from the complete immutable response bytes. A READY content URL never returns different bytes under the same content identity and ETag.

Media endpoints support `HEAD` with the same validators and representation metadata as `GET`, but without a body. They support one byte range per request:

- valid `Range: bytes=start-end` returns `206 Partial Content`;
- `Content-Range` identifies the exact interval and total length;
- malformed or unsatisfiable ranges return `416 Range Not Satisfiable` with `Content-Range: bytes */<total-length>`;
- Playback may send `If-Range` with the strong ETag;
- a mismatched `If-Range` returns the complete immutable representation rather than bytes that could be appended to stale cached data;
- multipart ranges are not required.

The MP4 asset uses fast-start layout so Playback can retrieve `moov` before sample ranges from `mdat`. Playback validates status, lengths, ranges, ETag, content identity, and manifest values before accepting bytes.

User Token issuance, source upload, owner listings, processing status, and failed-job endpoints use `Cache-Control: no-store`. Non-READY generated objects are never reachable through device routes. The first implementation proxies object bytes through FastAPI from the local Object Store; signed URLs and direct public object-store access are not required.
