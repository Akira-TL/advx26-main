# 12 — Retire the legacy package contract and document deployment

**What to build:** make the new Cloud Media Service the only supported backend contract, remove obsolete STL/WebM/package behavior, and provide verified OpenAPI and deployment guidance for the Sharing Interface and both fixed devices.

**Blocked by:** 10 — Serve authenticated Trigger and Playback contracts; 11 — Manage owned content, retry failures, and clean staging data.

**Status:** resolved

**Resolved by:** backend commit `6425e4a feat(backend): 完成云端媒体服务正式契约`

- [x] Legacy metadata/video/model/package/bundle routes and soft-delete behavior are absent from the active API.
- [x] Legacy schema, configuration, validators, tests, and embedded Swagger content have no active caller; generated FastAPI OpenAPI is canonical.
- [x] Health and readiness have distinct documented semantics and verify storage, media tools, device Tokens, renderer/Chromium, and worker when enabled.
- [x] Configuration documents User Token behavior, device Tokens, CORS, storage, limits, leases, retries, timeouts, cleanup, and binaries.
- [x] OpenAPI contains user, content, Trigger, Playback, HEAD, Range, If-Range, caching, and role-specific security contracts.
- [x] README examples cover Token issuance, upload, status polling, retry/delete, Trigger resolution, and Playback ranges with placeholders only.
- [x] Host and root-context multi-stage Docker deployment include FFmpeg/FFprobe, Node, renderer assets, Puppeteer, and Chromium dependencies.
- [x] The complete backend suite passes with 53 tests, including a full source-to-READY worker integration test.
- [x] CodeGraph finds no reachable legacy product symbols and all three security roles are documented in generated OpenAPI.
