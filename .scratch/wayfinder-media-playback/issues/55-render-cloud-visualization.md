# Render the selected visualization in the cloud

Type: grilling
Status: resolved
Blocked by: 08, 24, 28

## Question

Where does the video come from now that the product no longer selects a pre-provisioned clip, and how is the existing `Sound-Visualization-Kaleidoscope-effect` project used by the backend?

## Answer

The Cloud Media Service generates one visualization video for every uploaded Shared Sound by reusing the checked-in `Sound-Visualization-Kaleidoscope-effect/particle-field` WebGL2 renderer. There is no Preset Video pool and no user video upload in the competition path.

The existing browser application must gain a headless render mode that:

- accepts a normalized audio file instead of requesting microphone access;
- hides all recording, device, and debug controls;
- renders to a fixed 480x320 canvas using the existing WebGL2 implementation;
- advances from the media timeline and produces frames for exactly the normalized audio duration;
- uses a deterministic visual seed derived from the immutable `content_id` so retries reproduce the same visual;
- signals completion or failure to the backend process.

The backend runs the built renderer in Headless Chromium, captures a video-only render, then uses FFmpeg to produce the constrained fast-start H.264 MP4 required by `t5ai-h264-mp3-v1`. The normalized audio remains a separate `audio.mp3`; it is not multiplexed into `video.mp4`.

The renderer should target the final duration directly. FFmpeg may trim the captured video to correct small capture or encoder boundary differences, but looping a pre-existing clip is no longer the normal path. A pre-rendered fallback video is not part of the first accepted design.

Reimplementing the visualization in Python, replacing its shaders, rendering on the phone, and adding multiple selectable visual styles are outside the competition scope.
