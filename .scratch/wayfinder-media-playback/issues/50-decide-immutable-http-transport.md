# Decide immutable HTTP asset transport

Type: grilling
Status: resolved
Blocked by: 15, 17, 45, 47

## Question

What HTTP behavior must the Cloud Media Service provide for immutable media assets and Compact Content JSON?

## Answer

Every published Media Package revision is immutable and exposes a complete byte-range-capable HTTP contract. This applies to `video.mp4`, `audio.mp3`, `audio.idx`, the manifest, and the Compact Content JSON for that revision.

Each immutable response includes:

```http
Accept-Ranges: bytes
ETag: "<lowercase-file-sha256>"
Cache-Control: public, max-age=31536000, immutable
Content-Length: <exact-byte-length>
```

The ETag is a strong validator derived from the complete response bytes. A published URL must never later return different bytes under the same ETag or revision identity.

The service supports `HEAD` with the same validators and representation metadata as `GET`, but without a response body. It supports one byte range per request:

- a valid `Range: bytes=start-end` returns `206 Partial Content`;
- `Content-Range` identifies the exact returned interval and total resource length;
- an unsatisfiable or malformed range returns `416 Range Not Satisfiable` with `Content-Range: bytes */<total-length>`;
- Playback may send `If-Range` with the strong ETag when resuming or seeking;
- when `If-Range` does not match, the server returns the complete current representation rather than a partial body, and Playback must not concatenate it with cached bytes;
- multipart ranges are not required for the competition implementation.

The MP4 asset must use fast-start layout so Playback can retrieve `moov` before requesting sample ranges from `mdat`. Playback validates response status, `Content-Length`, `Content-Range`, ETag, expected revision, and manifest length before accepting bytes into an existing asset buffer. A validator mismatch is `CONTENT_INVALID`, not a transparent retry against the same immutable revision.

Dynamic creation, upload, processing, and status endpoints use `Cache-Control: no-store`. They are separate from published immutable revision URLs. A Compact Content URL is exposed only after atomic publication and thereafter follows the same immutable caching rules as the assets it references.
