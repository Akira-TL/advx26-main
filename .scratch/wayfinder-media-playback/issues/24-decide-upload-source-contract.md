# Decide the audio-only sharing source contract

Type: grilling
Status: resolved
Blocked by: 04, 08

## Question

What does a user submit when creating a shareable sound experience, and where does the video come from?

## Answer

Users share sound, not authored multimedia packages. The first backend contract accepts one raw audio recording or audio file and does not accept a user video, STL model, required title, or required description.

For the competition MVP, the Cloud Media Service pairs the submitted audio with one of several pre-provisioned cloud video files. It then produces equal-duration device-ready assets:

- an indexed JPEG frame stream for the Playback Board display;
- an independent MP3 asset that Playback decodes to PCM for the Speaker Link;
- a Compact Content URL that resolves to the Trigger-facing JSON document.

The Media Package remains limited to 30 seconds. The exact overlength policy and the pre-provisioned-video selection policy are separate decisions.

Future automatic visual generation is not part of the MVP. If implemented later, it uses an asynchronous generation job before the content becomes publishable.
