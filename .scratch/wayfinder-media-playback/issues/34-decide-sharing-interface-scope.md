# Decide the phone Sharing Interface scope

Type: grilling
Status: resolved
Blocked by: 29

## Question

Is the phone Sharing Interface part of this repository and current implementation scope?

## Answer

The Sharing Interface is external to this repository and is not part of the current development workstream. Another project or team owns recording/selecting a Shared Sound, uploading it, waiting for readiness, and writing the Compact Content URL to NFC.

This repository owns the backend API contract that the external client consumes. When the external code is synchronized later, integration work must conform to the immutable Compact Content URL and audio-only sharing decisions already recorded here; it must not reintroduce user-uploaded video, mandatory title/description fields, or a different manifest format.

For local testing, backend tests and small command-line fixtures may emulate the external client, but they are not a replacement mobile product.
