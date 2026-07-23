# Use synchronous cloud media preparation for the demo

Type: grilling
Status: resolved
Blocked by:

## Question

Where and when are device-ready media assets produced?

## Answer

The Cloud Media Service runs on a computer or server where FFmpeg and FFprobe can be installed.

For the competition MVP, content preparation is synchronous and limited to predictable operations:

- validate and normalize the submitted audio;
- pair it with a pre-provisioned cloud video;
- trim or loop the selected video to the final audio duration;
- transcode the video into constrained fast-start MP4/H.264;
- transcode the audio into an independent MP3 asset and generate `audio.idx`;
- publish an immutable Compact Content URL only after every asset is validated and ready.

The request may take longer, but the first implementation does not require a general asynchronous job system. A failed preparation receives a small fixed automatic retry budget; after two retries it enters `FAILED` and may be retried manually.

Future automatic video or visual generation is outside the MVP. If introduced later, generation runs asynchronously and must complete before the content can become `READY` or be written to NFC.
