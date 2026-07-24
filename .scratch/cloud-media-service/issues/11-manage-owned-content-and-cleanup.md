# 11 — Manage owned content, retry failures, and clean staging data

**What to build:** complete the user-owned lifecycle so a user can list and inspect their content, retry eligible failures, revoke or delete owned content, while the service safely cleans bounded temporary data without damaging source ownership or other users.

**Blocked by:** 03 — Run durable processing jobs with restart recovery; 09 — Publish the complete READY media package atomically.

**Status:** claimed

- [ ] A User Token can list and inspect only content owned by its resolved user identity across processing, READY, FAILED, and revoked/deleted states.
- [ ] Owner responses expose safe source metadata, normalized duration when known, current stage, timestamps, generated label, READY NFC URL, and stable failure information.
- [ ] An owner may retry only eligible FAILED content; retry reuses the same unpublished content identity, retained original source, and deterministic seed without duplicating published objects.
- [ ] Terminal invalid-source failures cannot be retried unless a new source upload creates a new content identity.
- [ ] Owner deletion or revocation immediately removes device access before physical generated-object cleanup occurs.
- [ ] One user cannot retry, revoke, delete, or inspect another user's content even when the content ID is known.
- [ ] Successful jobs clean private PCM, feature timeline, frame, and encoder intermediates after atomic publication.
- [ ] Failed-job diagnostic artifacts are retained only within configured age/size limits and never include plaintext tokens.
- [ ] Cleanup skips active leased jobs, permanent original source objects, immutable READY objects still referenced by active content, and all objects belonging to unrelated content.
- [ ] Lifecycle and cleanup behavior is covered by ASGI, repository, Object Store, restart, and failure-injection tests.
