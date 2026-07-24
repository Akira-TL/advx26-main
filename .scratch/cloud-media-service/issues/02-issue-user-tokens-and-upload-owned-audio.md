# 02 — Issue User Tokens and upload owned source audio

**What to build:** let the external Sharing Interface create a minimal user credential, authenticate later requests with it, upload one supported-size source audio object, and receive an owned immutable content identity plus initial processing status.

**Blocked by:** 01 — Establish the Cloud Media Service foundation.

**Status:** claimed

- [ ] A token-issuance request creates one User identity and returns one long-lived opaque User Token exactly once.
- [ ] SQLite stores a one-way token digest and metadata, never the recoverable plaintext token.
- [ ] A valid User Token resolves to its owner identity; missing, unknown, or malformed tokens receive a stable authentication error.
- [ ] An authenticated multipart upload accepts one non-empty source audio file up to 50 MiB without requiring metadata JSON, video, STL, title, or description.
- [ ] Upload permanently stores the exact original bytes in the Object Store before acknowledging success.
- [ ] Upload creates one immutable `content_id`, records the authenticated owner, creates an `UPLOADED` Processing Job, and returns content and status references without waiting for media processing.
- [ ] Users can list and query only their own newly uploaded content; another valid User Token cannot observe it.
- [ ] User-token, ownership, size-limit, empty-file, rollback, and source-byte integrity behavior is covered through FastAPI ASGI tests.
