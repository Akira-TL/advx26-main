# Implement constrained MP4 demux and sample source

Type: task
Status: resolved
Blocked by: 04, 05

## Goal

Open immutable `video.mp4`, parse the constrained MP4 metadata subset, and expose validated timestamped H.264 access units and sync-sample lookup without owning decoder or display behavior.

## Starting point

The current uncommitted Playback prototype contains generic HTTP download and PSRAM allocation work, but its JSON JPEG index, per-frame URLs, JPEG decoder, and elapsed-time scheduler are not part of this ticket or the final media contract.

## Functional scope

- verify a supported `ftyp` and reject fragmented MP4 (`moof`);
- locate and parse `moov` before media playback;
- select exactly one video `trak` whose handler and sample entry are supported H.264/AVC;
- parse the bounded metadata needed from `mvhd`, `tkhd`, `mdhd`, `hdlr`, `stsd`/`avc1`/`avcC`, `stts`, `stsc`, `stsz`, `stco` or `co64`, and `stss`;
- reject alternate video tracks, audio/subtitle tracks, B-frame composition-offset requirements, unsupported edit lists, unsupported sample entries, inconsistent counts, integer overflow, and out-of-file sample ranges;
- retain SPS/PPS and NAL length-size information from `avcC`;
- expose sample byte range, DTS/PTS, duration, and sync-sample status through a bounded lookup interface;
- locate the sync sample at or before a requested media position;
- retrieve exact sample ranges through `http_range_reader`;
- reconstruct one ordered H.264 access unit from MP4 length-prefixed NAL units in the framing expected by the decoder adapter;
- keep sample-table memory bounded and owned by the demux module;
- map invalid container or sample metadata to `MP4_DEMUX_FAILED` or `CONTENT_INVALID`.

## Module seam

Expose open, get codec configuration, locate sample, locate preceding sync sample, read access unit, and close operations. Hide MP4 box traversal, sample-table expansion, chunk mapping, byte offsets, and NAL length conversion.

## Completion criteria

- no separate `video.idx` or per-frame URL is required;
- all H.264 access-unit reads are derived from validated MP4 sample tables;
- seek callers can identify a valid sync sample and decode-forward range;
- the demux module has no dependency on H.264 decoder handles, LCD objects, MP3, or A2DP;
- the superseded JPEG prototype is not part of the functional video-source interface.

## Out of scope

H.264 decoding, color conversion, LCD presentation, MP3/A2DP, tests, build, flash, and hardware acceptance.

## Answer

Implemented constrained fast-start MP4 parsing, bounded video sample-table expansion, codec configuration extraction, sync/time lookup, Range-backed sample reads, container consistency checks, and length-prefixed NAL to Annex-B reconstruction in playback commit `67ff72a`.
