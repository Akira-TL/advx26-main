# Prototype headless cloud visualization rendering

Type: prototype
Status: open
Blocked by: 55, 56

## Question

Can the existing `Sound-Visualization-Kaleidoscope-effect/particle-field` WebGL2 renderer be adapted to consume a supplied audio file and reliably produce a deterministic 480x320, 10 fps, video-only render under Headless Chromium on the intended backend host?

## Prototype scope

Build the smallest throwaway path that answers the runtime question without implementing the production API or SQLite job system:

- load one local audio fixture instead of requesting microphone access;
- drive the existing audio-feature and renderer path from that file;
- hide the application controls and render at exactly 480x320;
- use a fixed seed and repeat the run to compare output identity;
- capture the complete visual duration under Headless Chromium;
- transcode the capture with FFmpeg into the initial `t5ai-h264-mp3-v1` H.264 profile;
- record wall-clock render time, peak memory, output duration, frame count, and any required Chromium flags.

## Acceptance

Resolve the ticket only after the prototype demonstrates or disproves all of the following:

- WebGL2 initializes without an interactive desktop session on the target host;
- file-based audio analysis drives the same visual feature model as microphone input;
- the captured output covers the normalized audio duration without missing the beginning or end;
- two runs with the same input and seed are sufficiently deterministic for retry behavior;
- FFmpeg can convert the capture into fast-start H.264 Baseline, YUV420P, 480x320, 10 fps, no B frames, and bounded GOP;
- one serial render stays within acceptable competition-server CPU and memory limits.

The production implementation must be based on the findings recorded here. If MediaRecorder timing is unreliable, the prototype should identify the smallest alternative capture seam, such as explicit frame stepping and frame-pipe encoding, rather than silently accepting drift.
