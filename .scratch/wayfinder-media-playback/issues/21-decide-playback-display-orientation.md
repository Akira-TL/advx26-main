# Decide the Playback Board media orientation

Type: grilling
Status: resolved
Blocked by: 06

## Question

Should the device-ready video and Playback Board experience use a 480x320 landscape canvas or a 320x480 portrait canvas on the rotated ILI9488 assembly?

## Answer

Use a 480x320 landscape device profile. The Cloud Media Service produces a 480x320 H.264 video track for the physical orientation already validated on the Playback Board, so the device does not rotate decoded video frames at runtime.
