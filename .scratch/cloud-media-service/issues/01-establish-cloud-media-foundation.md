# 01 — Establish the Cloud Media Service foundation

**What to build:** introduce the new content, job, media-object, and Object Store boundaries beside the legacy baseline so later vertical slices can persist user-owned source audio and immutable generated media without extending the obsolete package table.

**Blocked by:** None — can start immediately.

**Status:** resolved

- [x] SQLite initialization creates the new User, User Token, Content, Processing Job, and Media Object concepts with required indexes and constraints.
- [x] Existing legacy tables, when present, do not prevent startup; no automatic data conversion is required.
- [x] A filesystem Object Store accepts opaque keys and supports put/open/stat/range/delete/staging-promotion behavior without exposing arbitrary paths.
- [x] Object keys cannot escape the configured storage root through absolute paths or traversal segments.
- [x] Health remains process-only, while readiness verifies SQLite, Object Store writeability, and required configured storage roots.
- [x] Repository and Object Store tests use isolated temporary resources and assert externally observable behavior rather than SQL or path-construction details.
- [x] The current legacy API remains runnable until later tickets replace it, keeping the repository green after this prefactor.

**Implementation:** backend commit `e21bbb4` (`feat(storage): 建立云端媒体服务持久化基础`).
