# 03 — Run durable processing jobs with restart recovery

**What to build:** process uploaded content through one durable serial worker that claims SQLite jobs, publishes stage changes to owner status, retries only transient failures, and safely resumes work after process restart.

**Blocked by:** 02 — Issue User Tokens and upload owned source audio.

**Status:** resolved

**Resolved by:** backend commit `76208b7 feat(worker): 实现持久化媒体处理作业`

- [x] One in-process worker claims at most one eligible job at a time and prevents duplicate concurrent claims.
- [x] The owner-visible content status advances through stable stages including probing, audio normalization, visualization rendering, video encoding, index generation, validation, READY, and FAILED.
- [x] A processor seam allows tests to drive successful, terminal, transient, and interrupted outcomes without invoking real media binaries.
- [x] Terminal source errors fail immediately; configured transient failures retry only within a small fixed budget.
- [x] Claimed work abandoned by process termination becomes eligible for recovery through a lease or startup-reconciliation rule.
- [x] Repeating a completed job cannot create a second content identity or reclaim completed work.
- [x] Owner status exposes stable error code and safe message fields without leaking server paths, command lines, tokens, or unrelated content.
- [x] Worker startup and shutdown integrate with application lifespan without blocking health requests.
- [x] Claim exclusivity, retry classification, restart recovery, retry exhaustion, and stage reporting are covered by repository and ASGI tests.
