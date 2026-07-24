# Prototype headless cloud visualization rendering

Type: prototype
Status: resolved
Blocked by: 55, 56

## Question

Can the existing `Sound-Visualization-Kaleidoscope-effect/particle-field` WebGL2 renderer be adapted to consume a supplied audio file and reliably produce a deterministic 480x320, 10 fps, video-only render under Headless Chromium on the intended backend host?

## Prototype source

- Repository: `Sound-Visualization-Kaleidoscope-effect`
- Branch: `prototype/headless-cloud-render`
- Commit: `38dd584 feat(prototype): 验证无头云端可视化渲染`
- Command: `cd particle-field && npm run prototype:headless`

The throwaway prototype generates an eight-second deterministic WAV fixture, routes it through the existing `AudioAnalyzer`, renders the existing WebGL2 field in Puppeteer Chromium, records the Canvas with MediaRecorder, converts the result with FFmpeg, probes the H.264 output, and compares decoded frame hashes across two runs.

## Findings

Headless cloud rendering is technically viable:

- WebGL2 initialized without an interactive desktop session;
- Chromium used ANGLE over Vulkan SwiftShader;
- supplied file audio drove the existing audio-feature path without microphone hardware;
- the visual output was non-black and covered the source sound;
- FFmpeg produced fast-start MP4 with `moov` before `mdat`;
- the video was H.264 Constrained Baseline, YUV420P, 480x320, 10 fps, and contained no B frames;
- IDR/keyframes occurred every ten frames, or one second;
- one eight-second render completed in roughly nine seconds of end-to-end browser evaluation time.

The tested real-time capture seam is not suitable for production:

- an eight-second source produced an 8.3-second MP4 because the MediaRecorder tail and real-time scheduling extended the capture;
- the two outputs both contained 83 frames, but zero frame hashes matched at the same positions;
- rAF counts differed between runs, confirming scheduler-dependent animation advancement;
- the Chromium process tree peaked at approximately 1.23 GiB RSS under SwiftShader.

A fixed visual seed alone cannot make `requestAnimationFrame + canvas.captureStream + MediaRecorder` retry-safe because audio sampling, animation advancement, and capture timing remain wall-clock scheduled.

## Decision

Retain Headless Chromium and the existing WebGL2 shaders, but do not use MediaRecorder in the production Cloud Media Service.

The production render seam is:

1. FFmpeg decodes the normalized sound into a deterministic PCM representation.
2. The backend computes or reuses a deterministic, fixed-rate Audio Feature Timeline from that PCM. The timeline is an internal processing artifact, not a public media object.
3. The renderer accepts the feature timeline, immutable visual seed, frame index, and exact media time rather than reading `performance.now()` or live microphone state.
4. Chromium renders exactly `ceil(duration_seconds * 10)` frames at explicit timestamps `frame_index / 10` on a 480x320 canvas.
5. Each rendered frame is read through a deterministic Canvas/WebGL frame-export seam and piped directly to FFmpeg as ordered raw or lossless frames.
6. FFmpeg encodes the frame pipe into the constrained fast-start H.264 profile and trims only to the authoritative normalized audio duration.

This makes output duration independent of browser scheduling and makes retries reproducible from the same normalized audio, feature timeline, renderer version, and seed.

The backend initially runs one render job at a time because the measured software-rendering memory footprint is high. A later GPU-backed deployment may change performance, but it must not change the explicit-timeline contract.
