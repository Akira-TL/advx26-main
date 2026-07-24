# 12 — Retire the legacy package contract and document deployment

**What to build:** make the new Cloud Media Service the only supported backend contract, remove obsolete STL/WebM/package behavior, and provide verified OpenAPI and deployment guidance for the Sharing Interface and both fixed devices.

**Blocked by:** 10 — Serve authenticated Trigger and Playback contracts; 11 — Manage owned content, retry failures, and clean staging data.

**Status:** claimed

- [ ] Required metadata JSON, creator metadata in upload, user video, STL/model upload and validation, legacy package listing, model-file routes, ZIP bundle routes, and old soft-delete semantics are removed from the active API.
- [ ] Legacy schemas, configuration limits, validation helpers, storage assumptions, tests, Swagger artifacts, and documentation that have no remaining caller are deleted.
- [ ] Health and readiness document their distinct semantics; readiness checks SQLite, Object Store, FFmpeg, FFprobe, Node/renderer assets, Chromium launchability, and worker availability.
- [ ] Configuration documents User Token security behavior, fixed Trigger and Playback Tokens, CORS, storage roots, upload limits, worker lease/retry limits, process timeouts, cleanup bounds, and required binaries.
- [ ] OpenAPI describes token issuance, owned upload/list/status/retry/delete behavior, processing states and errors, Compact Content, playback descriptors, HEAD, Range, `If-Range`, cache headers, and role-specific authentication.
- [ ] Example commands demonstrate issuing a User Token, uploading audio, polling READY, resolving with Trigger Token, and downloading ranges with Playback Token without exposing real secrets.
- [ ] Docker or host deployment installs and validates FFmpeg/FFprobe, Node renderer dependencies, and Headless Chromium requirements.
- [ ] The complete new backend test suite passes from a clean environment, and no active test asserts the obsolete package contract.
- [ ] A final Standards and Spec review finds no reachable legacy product path or undocumented security role.
