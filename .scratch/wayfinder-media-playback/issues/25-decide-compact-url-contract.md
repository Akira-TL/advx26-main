# Decide the Compact Content URL contract

Type: grilling
Status: resolved
Blocked by: 10

## Question

Should the NFC URL directly return the complete Trigger-facing JSON document, redirect to a package manifest endpoint, or return only an identifier that requires a second fixed API request?

## Answer

The Compact Content URL directly returns the complete Trigger-facing JSON document with HTTP 200. The Trigger Board does not need to follow a redirect or perform a second identifier lookup.

Each published content revision receives an immutable Compact Content URL. Regenerating or republishing the same shared sound creates a new revision and a new URL instead of silently changing the document behind an already-written NFC tag.

The response separates:

- Trigger Control Panel data;
- the normalized Playback Session descriptor sent over the Board Link;
- revision and readiness metadata needed to reject incomplete content.

Only content in the `READY` state may be written to NFC and resolved for playback.
