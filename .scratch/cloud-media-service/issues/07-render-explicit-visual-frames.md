# 07 — Render explicit deterministic visual frames

**What to build:** adapt the selected WebGL2 visualization into a production headless renderer that consumes the feature timeline and seed, renders exact frame timestamps, and exports ordered 480x320 frames without real-time capture.

**Blocked by:** 06 — Build a deterministic Audio Feature Timeline.

**Status:** ready-for-agent

- [ ] The production renderer reuses the selected `Sound-Visualization-Kaleidoscope-effect` visual implementation rather than replacing it with another style.
- [ ] Headless mode never requests microphone access and does not require audible playback or an interactive desktop session.
- [ ] The renderer accepts explicit feature sample, immutable visual seed, frame index, media timestamp, output width, and output height.
- [ ] Animation and morph state advance from explicit media time; production output does not read `performance.now()` or depend on `requestAnimationFrame` frequency.
- [ ] The renderer produces exactly `ceil(duration_seconds * 10)` frames at timestamps `frame_index / 10` on a 480x320 canvas.
- [ ] Frames are exported through an ordered raw or lossless frame seam suitable for direct FFmpeg consumption; Canvas MediaRecorder is not used.
- [ ] Two runs with the same feature timeline, renderer version, seed, and configuration produce identical decoded frame hashes.
- [ ] Headless process launch uses bounded time, memory-aware serial execution, captured diagnostics, and reliable termination after success or failure.
- [ ] Integration tests cover short silence and mixed-feature timelines, first/last frame boundaries, exact frame count, deterministic identity, and non-black output.
- [ ] The implementation cites prototype branch `prototype/headless-cloud-render` commit `38dd584` as evidence, while not merging its real-time MediaRecorder path as production code.
