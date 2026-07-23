# Make the backend the media contract owner

Type: grilling
Status: resolved
Blocked by:

## Question

Which module owns the media package contract, and what legacy behavior is removed?

## Answer

The FastAPI module in the `backend/` Git submodule is the canonical Cloud Media Service and owns the manifest/API contract. Its current Swagger and implementation are a baseline to update, not a frozen contract.

The new contract accepts a user-provided Shared Sound, pairs it with a cloud-owned Preset Video, and publishes:

- one device-ready `video.mp4` containing a constrained H.264 video track;
- one independent `audio.mp3` asset plus `audio.idx` for precise audio seeking;
- immutable playback metadata for the `t5ai-h264-mp3-v1` profile.

User-video upload, STL upload, STL validation, bundle requirements for a model file, and local 3D rendering are removed from the product path. Authentication remains optional and disabled for the competition demo.
