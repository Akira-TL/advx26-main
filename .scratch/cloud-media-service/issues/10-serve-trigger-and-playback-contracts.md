# 10 — Serve authenticated Trigger and Playback contracts

**What to build:** let the fixed Trigger resolve NFC content and the fixed Playback retrieve immutable media through strict role-separated Bearer authentication, complete descriptors, and byte-range-capable HTTP responses.

**Blocked by:** 09 — Publish the complete READY media package atomically.

**Status:** ready-for-agent

- [ ] Server configuration requires separate fixed Trigger and Playback Tokens outside Git and maps them to stable roles.
- [ ] Token comparison is constant-time; missing, unknown, and wrong-role tokens are rejected before content or object access.
- [ ] A READY Compact Content URL returns the complete Trigger-facing JSON directly with HTTP 200 only for the Trigger role.
- [ ] The Compact Content response includes immutable `content_id`, duration, generated label, autoplay/end behavior, controls, and normalized Playback descriptors while omitting owner and source metadata.
- [ ] Playback asset endpoints require the Playback role and expose stable absolute URLs for video, audio, and audio index.
- [ ] Immutable responses provide exact Content-Length, strong SHA-derived ETag, `Cache-Control: private, max-age=31536000, immutable`, and `Vary: Authorization`.
- [ ] Media endpoints support full GET, HEAD, one prefix/open/suffix byte range, matching `If-Range`, mismatching `If-Range`, `206`, exact Content-Range, and correct `416` behavior.
- [ ] Non-READY, FAILED, deleted, missing, and corrupt-object content never returns playable descriptors or bytes.
- [ ] User Tokens cannot call Trigger or Playback routes, and device tokens cannot upload, list, retry, delete, or claim content ownership.
- [ ] ASGI tests cover the complete role matrix, descriptor schema, headers, body identity, Range behavior, and no cross-content byte concatenation.
