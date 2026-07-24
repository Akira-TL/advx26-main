# 07 — Render explicit deterministic visual frames

**What to build:** adapt the selected WebGL2 visualization into a production headless renderer that consumes the feature timeline and seed, renders exact frame timestamps, and exports ordered 480x320 frames without real-time capture.

**Blocked by:** 06 — Build a deterministic Audio Feature Timeline.

**Status:** resolved

**Resolved by:** visualization commit `69a700d feat(render): 实现确定性显式帧渲染`

- [x] The renderer reuses the latest selected `Sound-Visualization-Kaleidoscope-effect` implementation.
- [x] Headless mode uses only supplied feature data and never requests microphone or audible playback.
- [x] It accepts explicit features, seed, frame index, timestamp, width, and height.
- [x] Production `renderAt/updateAt` paths advance only from explicit media time.
- [x] Exactly one ordered 480x320 lossless PNG is emitted for every 10 fps timeline frame.
- [x] Canvas MediaRecorder is absent from the production path.
- [x] Repeated runs produce identical WebGL pixel hashes and PNG SHA-256 values.
- [x] Puppeteer launch is bounded, serial, diagnostics-capturing, and reliably terminated.
- [x] Integration tests cover silent and mixed features, boundaries, frame count, non-black output, and deterministic identity.
- [x] The production summary cites prototype evidence commit `38dd584` without reusing its MediaRecorder path.
