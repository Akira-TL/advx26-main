# 11 — Manage owned content, retry failures, and clean staging data

**What to build:** complete the user-owned lifecycle so a user can list and inspect their content, retry eligible failures, revoke or delete owned content, while the service safely cleans bounded temporary data without damaging source ownership or other users.

**Blocked by:** 03 — Run durable processing jobs with restart recovery; 09 — Publish the complete READY media package atomically.

**Status:** resolved

**Resolved by:** backend commit `d3fffb0 feat(content): 实现用户内容生命周期与清理`

- [x] User Tokens list and inspect only their own content across processing, READY, FAILED, and DELETED states.
- [x] Owner responses expose source metadata, duration, stage, timestamps, label, NFC URL, and safe failures.
- [x] Eligible transient failures retry with the same content, source, and deterministic seed.
- [x] Terminal source failures return conflict and require a new upload.
- [x] Owner deletion immediately removes Trigger/Playback access before physical cleanup.
- [x] Cross-user inspect, retry, and delete operations return not found.
- [x] Successful publication best-effort removes private job staging.
- [x] Failed diagnostics are bounded by age and aggregate bytes.
- [x] Cleanup skips active leases, permanent sources, immutable final objects, and unrelated staging prefixes.
- [x] ASGI and cleanup tests cover retry, deletion, role isolation, active work, and source preservation.
