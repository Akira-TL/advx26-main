# Decide media indexes and integrity metadata

Type: grilling
Status: resolved
Blocked by: 14, 15, 16, 17

## Question

How should Playback locate JPEG and MP3 frames for streaming, seeking, and corruption detection?

Decide whether `video.idx` and an optional `audio.idx` are fixed-width binary or JSON, whether MP3 seek uses a server-generated frame index or only a CBR byte estimate, and which integrity values are available before or during playback.

## Answer

The `t5ai-jpeg-mp3-v1` profile publishes fixed-width little-endian binary indexes for both media assets. Playback must not infer seek offsets from a CBR byte ratio alone.

Both index files start with a 16-byte header:

```text
0–3     magic (`VJX1` for video, `AIX1` for audio)
4–5     index_version = 1
6–7     record_size = 16
8–11    record_count
12–15   reserved = 0
```

Each `video.idx` record is 16 bytes:

```text
0–3     pts_ms
4–7     byte_offset in video.mjpg
8–11    byte_length of the complete JPEG frame
12–15   crc32 of the indexed JPEG bytes
```

Each `audio.idx` record is 16 bytes and describes one complete MP3 frame:

```text
0–3     decoded_pcm_sample_position
4–7     byte_offset in audio.mp3
8–11    byte_length of the complete MP3 frame
12–15   crc32 of the indexed MP3 bytes
```

Indexes use monotonically increasing presentation/sample positions and non-overlapping byte ranges fully contained by their corresponding asset. The Cloud Media Service validates these invariants before publication.

For an out-of-buffer MP3 seek, Playback selects the indexed frame at or before the target, starts decoding from a short preroll window of up to ten preceding MP3 frames, and discards decoded samples until the requested sample position. This avoids relying on a simple CBR estimate and gives the decoder enough history for Layer III frame dependencies.

The manifest includes, for each media and index asset:

- exact byte length;
- lowercase hexadecimal SHA-256;
- immutable ETag or equivalent revision identity;
- index version where applicable.

Playback validates Content-Length, ETag/revision identity, index bounds, and per-record CRC32 while streaming. Whole-file SHA-256 is mandatory during backend publication and may also be checked by Playback only when the complete asset is locally available; playback does not block on downloading a whole asset solely to recompute SHA-256.
