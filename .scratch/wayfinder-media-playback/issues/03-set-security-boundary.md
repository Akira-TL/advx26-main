# Set the competition security boundary

Type: grilling
Status: resolved
Blocked by:

## Question

What security boundary applies to the competition prototype?

## Answer

The competition backend uses simple Bearer Token authentication rather than anonymous access or production-grade identity infrastructure.

Three credential classes exist:

- a backend-issued opaque User Token identifies the user who uploads and owns source audio and generated content;
- one fixed Trigger Token identifies the single competition Trigger Board and authorizes Compact Content resolution;
- one fixed Playback Token identifies the single competition Playback Board and authorizes playback descriptor and media-object retrieval.

User Token digests and content ownership are recorded in SQLite. The fixed device tokens are deployment secrets supplied through Git-ignored configuration and are not issued, refreshed, rotated, or paired by the backend during the competition build.

Every protected endpoint rejects a missing token, an unknown token, or a token from the wrong role. Tokens are compared or verified without storing recoverable plaintext user credentials. The design does not add passwords, email verification, JWT infrastructure, signed URLs, device attestation, per-session grants, automatic token rotation, abuse prevention, or hostile-network hardening.
