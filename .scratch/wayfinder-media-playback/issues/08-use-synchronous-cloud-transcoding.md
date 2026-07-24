# Use durable SQLite media preparation for the demo

Type: grilling
Status: resolved
Blocked by: 04

## Question

Where and when are device-ready media assets produced?

## Answer

The Cloud Media Service runs on a computer or server where FFmpeg, FFprobe, Node.js, and Headless Chromium can be installed. It owns all media preparation rather than requiring the phone or either board to pre-process content.

Because rendering the visualization runs for approximately the source duration and may approach 30 seconds, the upload request does not remain open until the package is READY. It authenticates the user, stores the source object, creates the owned content record and a durable SQLite Processing Job, then returns the content identity and status endpoint.

A single in-process media worker initially performs the pipeline:

- inspect and repair the submitted source audio;
- normalize it into the independent MP3 asset;
- run `Sound-Visualization-Kaleidoscope-effect` in headless render mode for the normalized duration;
- transcode the captured visual into constrained fast-start MP4/H.264;
- generate `audio.idx`;
- validate every published object and manifest field;
- promote the content to `READY` only when the complete package is available.

Invalid input fails without retry. Transient renderer, encoder, or storage errors receive a small bounded retry budget. SQLite retains job stage and attempt state so an interrupted job can be reclaimed after process restart. A general distributed job system remains out of scope.
