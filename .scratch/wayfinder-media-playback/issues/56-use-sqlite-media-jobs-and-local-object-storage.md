# Use SQLite media jobs and simple local object storage

Type: grilling
Status: resolved
Blocked by: 08, 47, 55

## Question

How does the competition backend persist source and generated objects, execute long-running cloud rendering, and recover after process restarts without introducing external infrastructure?

## Answer

The Cloud Media Service remains one deployable FastAPI application backed by SQLite and a simple filesystem object store. It does not require Redis, a message broker, PostgreSQL, S3, or a separate worker service for the competition build.

The upload request creates an owned content record and a durable SQLite Processing Job, then returns without waiting for the complete render. One in-process media worker claims jobs from SQLite and advances them through explicit stages such as:

- `UPLOADED`;
- `PROBING`;
- `NORMALIZING_AUDIO`;
- `RENDERING_VIDEO`;
- `ENCODING_VIDEO`;
- `BUILDING_INDEX`;
- `VALIDATING`;
- `READY` or `FAILED`.

Only one media job runs at a time initially so concurrent Chromium and FFmpeg processes cannot exhaust the demo server. The headless-render prototype measured approximately 1.23 GiB peak RSS for the Chromium process tree under SwiftShader, so serial rendering is an explicit competition constraint rather than only a conservative default. Jobs abandoned by a process restart become claimable again. Invalid source media fails without retry; transient renderer, encoder, or storage failures receive a small bounded retry budget.

Storage is exposed to application code through a small Object Store interface rather than arbitrary path manipulation. The first implementation maps object keys onto a server-local directory. It permanently retains each original uploaded audio object and the immutable READY outputs:

- `video.mp4`;
- `audio.mp3`;
- `audio.idx`;
- `manifest.json`.

Temporary work objects live under a private staging keyspace. They may include normalized PCM, the deterministic Audio Feature Timeline, frame-pipe scratch data, FFmpeg logs, and unpublished encoded outputs. Successful jobs remove their temporary intermediates; failed jobs may retain bounded diagnostic artifacts until a cleanup operation removes them. A future S3-compatible adapter may replace the filesystem implementation without changing the media-domain or API contracts, but that adapter is not required for the competition build.
