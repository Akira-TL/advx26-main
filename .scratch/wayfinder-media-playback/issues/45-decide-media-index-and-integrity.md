# Decide media indexes and integrity metadata

Type: grilling
Status: resolved
Blocked by: 14, 15, 16, 17

## Question

How should Playback locate MP4/H.264 and MP3 data for streaming, seeking, and corruption detection?

## Answer

The `t5ai-h264-mp3-v1` profile does not publish a separate video index. Playback obtains video timing, byte offsets, sample sizes, sync samples, and codec initialization data from the MP4 metadata.

The server must produce a bounded MP4 layout:

- `moov` precedes `mdat`;
- exactly one H.264 video track is present;
- required sample tables are complete and internally consistent;
- `avcC` contains valid SPS/PPS and NAL length-size information;
- sync samples/IDR positions are available for seek and recovery;
- DTS and PTS are monotonic and bounded by the declared duration;
- sample byte ranges are fully contained within the immutable `video.mp4` asset.

Playback parses only the MP4 box and sample-table subset needed by this constrained profile. It rejects unsupported track layouts, fragmented MP4, edit lists that alter the media timeline, B-frame composition reordering, unknown sample entries, and out-of-bounds sample ranges.

The independent MP3 asset retains a fixed-width little-endian `audio.idx`. Its 16-byte header is:

```text
0–3     magic `AIX1`
4–5     index_version = 1
6–7     record_size = 16
8–11    record_count
12–15   reserved = 0
```

Each 16-byte audio record contains:

```text
0–3     decoded_pcm_sample_position
4–7     byte_offset in audio.mp3
8–11    byte_length of the complete MP3 frame
12–15   crc32 of the indexed MP3 bytes
```

For an out-of-buffer video seek, Playback chooses the MP4 sync sample at or before the target, resets the H.264 decoder, reapplies SPS/PPS, decodes forward, and suppresses presentation until the requested timestamp. For MP3 seek, Playback selects the indexed frame at or before the target, starts from a short preroll window of up to ten preceding MP3 frames, and discards decoded samples until the requested position.

The manifest includes exact byte length, lowercase hexadecimal SHA-256, and immutable ETag for `video.mp4`, `audio.mp3`, and `audio.idx`; `audio.idx` also carries its index version. Playback validates resource identity, MP4 sample bounds, audio index bounds, and per-record MP3 CRC32 while streaming. Whole-file SHA-256 remains mandatory during backend publication and is optional on-device unless the full asset is already available.
