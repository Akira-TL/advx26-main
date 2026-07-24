# Decide the Shared Sound duration policy

Type: grilling
Status: resolved
Blocked by: 06, 24, 55

## Question

When a submitted Shared Sound exceeds 30 seconds, what portion is retained, and how is the generated visual aligned to the final sound duration?

## Answer

The Cloud Media Service automatically retains the first 30 seconds when a Shared Sound exceeds the competition limit. It does not require an interactive trimming step. Shorter sounds retain their complete decoded duration without padding.

The normalized MP3 duration becomes the authoritative Media Package duration. `Sound-Visualization-Kaleidoscope-effect` renders against that media timeline and targets the same duration directly. FFmpeg may trim small capture or encoder boundary excess so the constrained H.264 video and independent MP3 remain within the agreed synchronization tolerance, but the normal path does not loop a pre-existing video.

The Compact Content response reports the normalized duration. The backend retains the original source audio permanently and may record its original probed duration for diagnostics and ownership history, but the boards only consume the normalized duration.
