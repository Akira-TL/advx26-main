# Make the backend the media contract owner

Type: grilling
Status: resolved
Blocked by:

## Question

Which component owns the media package contract, and what legacy behavior is removed?

## Answer

The FastAPI service in the `backend/` Git submodule is the canonical Cloud Media Service and owns the manifest/API contract. Its current Swagger and implementation are a baseline to update, not a frozen contract.

The new contract accepts a user-provided Shared Sound, pairs it with a cloud-owned Preset Video, and publishes an indexed JPEG frame stream, an independent MP3 audio asset, and playback metadata. User-video upload, STL upload, STL validation, bundle requirements for a model file, and local 3D rendering are removed from the product path. Authentication remains optional and disabled for the competition demo.
