# Implement immutable HTTP range reader

Type: task
Status: resolved
Blocked by: 01

## Goal

Provide one bounded immutable-resource reader shared by MP4 demux, H.264 sample retrieval, MP3 retrieval, and audio-index loading.

## Functional scope

- perform `HEAD` to confirm `Content-Length`, strong ETag, `Accept-Ranges: bytes`, and expected resource identity;
- issue single byte-range GET requests and require valid `206 Content-Range` responses;
- send `If-Range` with the expected strong ETag when continuing or seeking;
- reject full or partial responses whose ETag, length, range, or revision identity differs from the descriptor;
- implement three retries with approximately 250 ms, 500 ms, and 1 s backoff;
- return exact byte counts and normalized network/range errors;
- allow callers to cancel outstanding reads when a new session replaces the current one or STOP occurs;
- keep network buffers bounded and caller-owned data delivery explicit;
- support small metadata reads near the MP4 head and arbitrary sample reads from `mdat` without exposing HTTP callback details to demux/decoder code.

## Module seam

Expose open/verify, read-range, cancel, and close operations over one immutable asset descriptor. Callers provide the destination buffer or bounded sink; the reader owns no media interpretation.

## Completion criteria

- MP4, MP3, and audio-index modules share one transport implementation;
- bytes from different ETags or resource revisions can never be concatenated;
- invalid ranges map to `NETWORK_RANGE_INVALID`; retry exhaustion maps to `NETWORK_TIMEOUT`;
- MP4 metadata and sample reads use the same verified asset identity.

## Out of scope

MP4 parsing, codec decoding, scheduling, tests, build, flash, and server-side Range implementation.

## Answer

Implemented immutable HEAD/Range probing, strong ETag and length verification, `If-Range`, exact `206 Content-Range` validation, bounded caller-owned reads, cancellation, and stable transport error mapping in playback commit `721a793`.
