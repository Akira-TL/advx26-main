# Decide the phone Sharing Interface scope

Type: grilling
Status: resolved
Blocked by: 29, 53

## Question

Is the phone Sharing Interface part of this repository and current implementation scope?

## Answer

The Sharing Interface remains external to this repository and is not part of the current backend and firmware development workstream. Another project owns recording or selecting a Shared Sound, requesting and persisting the user's opaque User Token, uploading the source audio, polling the owned processing state, and writing the READY Compact Content URL to NFC.

This repository owns the backend contract consumed by that client, including User Token issuance and recognition, user-content ownership, source-audio upload, asynchronous processing state, immutable content publication, and the NFC URL returned after readiness.

When the external client is synchronized later, it must not reintroduce user-uploaded video, STL data, mandatory creator-authored title or description, synchronous media rendering in the upload request, or a different ownership model. Backend tests and small command-line fixtures may emulate the external client for local development.
