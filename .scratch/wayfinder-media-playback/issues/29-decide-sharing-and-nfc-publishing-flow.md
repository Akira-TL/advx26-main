# Decide the Shared Sound submission and NFC publishing flow

Type: grilling
Status: resolved
Blocked by: 24, 25, 53

## Question

How does a user submit a Shared Sound, retain ownership, and publish the resulting content to NFC?

## Answer

Use a phone-facing Sharing Interface. The client first obtains and persists a backend-issued opaque User Token. The user then records a sound or selects an existing audio file and uploads it with that token. The Cloud Media Service identifies the user, permanently stores the original audio under that owner, creates an asynchronous media-processing job, and returns the new `content_id` and status endpoint.

The client polls the owned content status until it becomes `READY`. It then obtains the immutable Compact Content URL and writes that URL to one or more NFC tags with the same phone. Writing a tag does not transfer ownership, create a new cloud record, or require the backend to know the physical tag UID.

The Trigger Board does not record, upload, own, or write NFC content. For competition reliability, known-good NFC tags may be prepared in advance, but that is a fallback demonstration procedure rather than the canonical product flow.
