# Implement Playback recovery and resynchronization

Type: task
Status: open
Blocked by: 10

## Goal

Apply the agreed deterministic recovery policies across Board Link, immutable HTTP, MP4 demux, H.264 decode, MP3 decode, Speaker Link, and session replacement.

## Functional scope

- preserve active media execution across Board Link disconnect;
- answer reconnecting `GET_STATUS` from the current authoritative snapshot;
- expose boot ID so Trigger can distinguish reconnect from Playback reboot;
- accept reload/seek restoration after reboot or lost in-memory session;
- apply HTTP retry exhaustion and immutable-resource mismatch as distinct retryable/fatal errors;
- fail invalid MP4 metadata, unsupported track layout, invalid codec configuration, or out-of-bounds samples as `MP4_DEMUX_FAILED` or `CONTENT_INVALID`;
- after an isolated H.264 decode failure, suppress invalid presentation, reset at the next valid IDR, reapply SPS/PPS, and decode forward;
- fail after two consecutive GOP recovery failures or inability to resume from a valid IDR within the agreed bounded interval;
- resynchronize isolated MP3 errors with timeline-preserving silence and fail after the agreed threshold;
- transition to `WAITING_SPEAKER` on Speaker Link loss, freeze position, and resume only when prior intent was `PLAYING`;
- keep `PAUSED` intent across speaker loss and reconnection;
- cancel old network reads and discard old MP4/H.264/MP3 decoder output when a replacement session arrives;
- ensure stale asynchronous callbacks cannot display frames, consume PCM, or report against a newer session generation.

## Module seam

Recovery remains coordinated by `playback_engine` using normalized media-pipeline events. Transport, demux, decoders, and Speaker Link report typed failures rather than directly changing Trigger-visible state.

## Completion criteria

- every recoverable interruption has one deterministic state transition and resume rule;
- immutable content corruption cannot enter an automatic retry loop;
- an invalid H.264 reference chain is never presented after a decoder error;
- stale work from a replaced session cannot display frames, consume PCM, or emit reports;
- fatal errors retain enough snapshot information for Trigger to show the agreed error state.

## Out of scope

Tests, build, flash, physical pairing, Bluetooth coexistence validation, and performance tuning.
