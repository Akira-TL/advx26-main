# 06 — Build a deterministic Audio Feature Timeline

**What to build:** derive one fixed-rate private feature timeline from normalized PCM so the selected visualization receives reproducible volume, frequency-band, centroid, flux, transient, and onset values without live browser audio sampling.

**Blocked by:** 04 — Probe, repair, and normalize source audio.

**Status:** resolved

**Resolved by:** backend commits `3b95a8e feat(render): 生成确定性音频特征时间线` and `26d6a67 fix(render): 补全可视化音高特征`

- [x] Feature extraction consumes deterministic normalized PCM rather than live browser audio or wall-clock callbacks.
- [x] The timeline defines a stable version, 10 fps cadence, authoritative duration, and the complete updated visualization vocabulary including pitch fields.
- [x] Windows and timestamps are derived from PCM sample positions.
- [x] The timeline contains one sample for every 10 fps video frame.
- [x] Silence, bands, centroid, flux, onset, transient, and pitch behavior remain compatible with the selected analyzer intent.
- [x] Identical PCM produces byte-identical timeline output and SHA-256.
- [x] PCM or algorithm-version changes produce a different timeline identity.
- [x] The timeline is stored only as a private staging artifact.
- [x] Tests cover silence, steady tones, frequency separation, impulses, duration boundaries, pitch, and repeated-run identity.
