# Decide the audio-only sharing source contract

Type: grilling
Status: resolved
Blocked by: 04, 08, 53

## Question

What does a user submit when creating a shareable sound experience, and where does the video come from?

## Answer

An authenticated user submits one raw audio recording or audio file. The backend records that user as the owner and permanently retains the original uploaded audio object. The first contract does not accept a user video, STL model, required title, or required description.

The Cloud Media Service derives the visual from the submitted sound by running the checked-in `Sound-Visualization-Kaleidoscope-effect` renderer in headless mode. It produces equal-duration device-ready assets:

- one constrained fast-start `video.mp4` containing the generated H.264 visual track;
- one independent normalized `audio.mp3` plus `audio.idx` for Playback;
- one manifest and Compact Content URL for the immutable READY content item.

The normalized content duration is at most 30 seconds. The video renderer targets that exact duration, while FFmpeg may trim small capture-boundary excess. There is no normal-path Preset Video pool and no user-selectable visual in the competition build.
