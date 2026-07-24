# 09 — Publish the complete READY media package atomically

**What to build:** validate the independently completed audio and video chains, create one immutable device manifest, promote the complete generated object set, and expose the content as READY in one coordinated publication step.

**Blocked by:** 05 — Generate and validate the indexed MP3 asset; 08 — Encode and validate the constrained H.264 video.

**Status:** ready-for-agent

- [ ] Publication requires validated `video.mp4`, `audio.mp3`, `audio.idx`, normalized duration, generated display label, and all integrity metadata.
- [ ] Manifest uses schema version 1, immutable `content_id`, state READY, `t5ai-h264-mp3-v1`, Trigger presentation data, and normalized Playback descriptors.
- [ ] Video and audio durations agree within the decided competition tolerance and never exceed 30 seconds.
- [ ] Every manifest URL, object key, length, SHA-256, ETag, codec field, and index version refers to the same content identity and generated object set.
- [ ] Generated files remain inaccessible in a private staging keyspace until all validation passes.
- [ ] The filesystem Object Store promotes the complete set atomically where supported, and SQLite changes to READY only after final objects are present.
- [ ] A failure before or during promotion leaves device routes unable to observe any partial package and records a recoverable or terminal processing failure as appropriate.
- [ ] READY content and its objects are immutable; regeneration creates another content identity rather than overwriting bytes.
- [ ] Original owner and source-object relationships remain intact after publication.
- [ ] Failure-injection tests cover each publication boundary and prove that observable state is either non-READY or one complete READY package.
