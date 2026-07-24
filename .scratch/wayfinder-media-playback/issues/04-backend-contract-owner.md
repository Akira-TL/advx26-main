# Make the backend the media contract owner

Type: grilling
Status: resolved
Blocked by:

## Question

Which module owns the media package contract, and what legacy behavior is removed?

## Answer

The FastAPI module in the `backend/` Git submodule is the canonical Cloud Media Service and owns the user-token, upload, processing-state, object-storage, Compact Content, manifest, and authenticated media-delivery contracts. Its current Swagger and implementation are a baseline to replace, not a frozen contract.

The new contract accepts one raw Shared Sound from an authenticated user, permanently retains that original source audio, and performs the complete cloud media pipeline:

- inspect and repair supported input media with FFprobe and FFmpeg;
- normalize the audio and retain at most the first 30 seconds;
- render the selected `Sound-Visualization-Kaleidoscope-effect` visual in Headless Chromium;
- transcode the captured visual into constrained fast-start MP4/H.264;
- publish one independent `audio.mp3` plus `audio.idx`;
- validate and atomically expose one immutable READY content item backed by simple object storage and SQLite records.

User-video upload, STL upload, STL validation, model files, ZIP bundle requirements, local 3D rendering, and a Preset Video pool are removed from the product path.
