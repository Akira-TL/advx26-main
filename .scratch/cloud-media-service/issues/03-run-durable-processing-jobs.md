# 03 — Run durable processing jobs with restart recovery

**What to build:** process uploaded content through one durable serial worker that claims SQLite jobs, publishes stage changes to owner status, retries only transient failures, and safely resumes work after process restart.

**Blocked by:** 02 — Issue User Tokens and upload owned source audio.

**Status:** claimed

- [ ] One in-process worker claims at most one eligible job at a time and prevents duplicate concurrent claims.
- [ ] The owner-visible content status advances through stable stages including probing, audio normalization, visualization rendering, video encoding, index generation, validation, READY, and FAILED.
- [ ] A processor seam allows tests to drive successful, terminal, transient, and interrupted outcomes without invoking real media binaries.
- [ ] Terminal source errors fail immediately; configured transient failures retry only within a small fixed budget.
- [ ] Claimed work abandoned by process termination becomes eligible for recovery through a lease or startup-reconciliation rule.
- [ ] Repeating a claimed step is idempotent with respect to published object metadata and does not create a second content identity.
- [ ] Owner status exposes stable error code and safe message fields without leaking server paths, command lines, tokens, or unrelated content.
- [ ] Worker startup and shutdown integrate with application lifespan without blocking health requests.
- [ ] Claim exclusivity, retry classification, restart recovery, and stage reporting are covered by repository and ASGI tests.
