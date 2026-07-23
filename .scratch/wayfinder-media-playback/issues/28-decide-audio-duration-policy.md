# Decide the Shared Sound duration policy

Type: grilling
Status: resolved
Blocked by: 06, 24

## Question

When a submitted Shared Sound exceeds 30 seconds, should the backend reject it, automatically keep the first 30 seconds, or require the user to choose a segment? Also confirm that shorter Preset Videos may be looped and longer ones trimmed to the final sound duration.

## Answer

The Cloud Media Service automatically keeps the first 30 seconds when a Shared Sound is longer than the MVP limit. It does not require an interactive trimming step.

For shorter sounds, retain the complete sound. The selected Preset Video is looped when shorter than the normalized sound and trimmed when longer. The generated H.264 video track and independent MP3 asset must have the same final duration within the agreed synchronization tolerance.

The Compact Content URL response reports the normalized duration. The original uploaded duration may be retained as backend diagnostics but is not required by either board.
