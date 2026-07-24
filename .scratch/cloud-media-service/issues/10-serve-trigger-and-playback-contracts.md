# 10 — Serve authenticated Trigger and Playback contracts

**What to build:** let the fixed Trigger resolve NFC content and the fixed Playback retrieve immutable media through strict role-separated Bearer authentication, complete descriptors, and byte-range-capable HTTP responses.

**Blocked by:** 09 — Publish the complete READY media package atomically.

**Status:** resolved

**Resolved by:** backend commit `4ce4df1 feat(device): 实现设备鉴权与媒体分发`

- [x] Separate fixed Trigger and Playback Tokens are loaded from deployment configuration and mapped to stable roles.
- [x] Constant-time comparison and strict missing, unknown, and wrong-role rejection occur before object access.
- [x] Trigger resolves complete READY Compact Content directly through HTTP 200.
- [x] Compact Content exposes presentation/playback data and omits owner/source metadata.
- [x] Playback receives absolute video, audio, and index URLs and role-protected immutable bytes.
- [x] Responses include exact length, strong ETag, immutable private caching, and Authorization variance.
- [x] GET, HEAD, prefix/open/suffix Range, If-Range, 206, Content-Range, and 416 are implemented.
- [x] Non-READY, missing, deleted, and corrupt objects never return playable content.
- [x] User and device roles cannot cross into each other's APIs.
- [x] ASGI tests cover role matrix, schema, headers, exact bytes, ranges, and corruption rejection.
