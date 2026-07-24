# 06 — Build a deterministic Audio Feature Timeline

**What to build:** derive one fixed-rate private feature timeline from normalized PCM so the selected visualization receives reproducible volume, frequency-band, centroid, flux, transient, and onset values without live browser audio sampling.

**Blocked by:** 04 — Probe, repair, and normalize source audio.

**Status:** claimed

- [ ] Feature extraction consumes deterministic normalized PCM rather than a microphone, audio element, real-time AnalyserNode, or wall-clock callback.
- [ ] The timeline defines a stable version, sample cadence, authoritative duration, and the complete feature vocabulary required by the existing visualization renderer.
- [ ] Feature windows and timestamps are derived from PCM sample positions, not process scheduling.
- [ ] The timeline covers every 10 fps video frame through a defined interpolation or nearest-sample rule.
- [ ] Silence, low-level input, frequency bands, centroid, spectral flux, transient decay, and onset behavior remain compatible with the visual intent of the current analyzer.
- [ ] Repeating extraction from identical PCM produces byte-identical timeline output and SHA-256.
- [ ] A changed PCM input or changed feature-algorithm version produces a different declared identity rather than silently reusing incompatible data.
- [ ] The timeline is stored only as a private staging artifact or deterministic recomputable value and is never exposed in user, Trigger, or Playback APIs.
- [ ] Golden fixture tests cover silence, steady tones, mixed bands, impulses, duration boundaries, and repeated-run identity.
