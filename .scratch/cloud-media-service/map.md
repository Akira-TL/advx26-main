# Cloud Media Service Implementation Map

## Specification

- [Cloud Media Service Implementation Specification](spec.md)

## Dependency graph

```text
01 Cloud Media foundation
└── 02 User Tokens and owned upload
    └── 03 Durable processing jobs
        └── 04 Source probing and audio normalization
            ├── 05 Indexed MP3 asset ─────────────────────┐
            └── 06 Deterministic Audio Feature Timeline   │
                └── 07 Explicit WebGL visual frames       │
                    └── 08 Constrained H.264 video ───────┤
                                                         └── 09 Atomic READY publication
                                                             ├── 10 Trigger/Playback delivery ─┐
                                                             └── 11 Owner lifecycle/cleanup ───┤
                                                                                               └── 12 Retire legacy contract
```

## Tickets

- [01 — Establish the Cloud Media Service foundation](issues/01-establish-cloud-media-foundation.md)
- [02 — Issue User Tokens and upload owned source audio](issues/02-issue-user-tokens-and-upload-owned-audio.md)
- [03 — Run durable processing jobs with restart recovery](issues/03-run-durable-processing-jobs.md)
- [04 — Probe, repair, and normalize source audio](issues/04-normalize-source-audio.md)
- [05 — Generate and validate the indexed MP3 asset](issues/05-generate-indexed-mp3-asset.md)
- [06 — Build a deterministic Audio Feature Timeline](issues/06-build-deterministic-audio-feature-timeline.md)
- [07 — Render explicit deterministic visual frames](issues/07-render-explicit-visual-frames.md)
- [08 — Encode and validate the constrained H.264 video](issues/08-encode-and-validate-h264-video.md)
- [09 — Publish the complete READY media package atomically](issues/09-publish-ready-media-package-atomically.md)
- [10 — Serve authenticated Trigger and Playback contracts](issues/10-serve-trigger-and-playback-contracts.md)
- [11 — Manage owned content, retry failures, and clean staging data](issues/11-manage-owned-content-and-cleanup.md)
- [12 — Retire the legacy package contract and document deployment](issues/12-retire-legacy-contract-and-document-deployment.md)

## Frontier

- Ticket 01 is resolved by backend commit `e21bbb4`.
- Ticket 02 is resolved by backend commit `f1179a2`.
- Ticket 03 is resolved by backend commit `76208b7`.
- Ticket 04 is resolved by backend commit `94d000e`.
- Ticket 05 is resolved by backend commit `c1c7c8a`.
- Ticket 06 is claimed and is the current implementation frontier.
- Ticket 09 requires both the indexed-audio and encoded-video branches.
- Tickets 10 and 11 may proceed in parallel after publication and job prerequisites are satisfied.
- Ticket 12 is the final contract and documentation cleanup after both device delivery and owner lifecycle are complete.

## Repository boundaries

- The canonical service implementation and tests live in the `backend/` Git submodule.
- Ticket 07 necessarily ports validated renderer changes into the `Sound-Visualization-Kaleidoscope-effect` repository or a production renderer artifact consumed by the backend. The throwaway MediaRecorder branch is evidence, not production code.
- Root `.scratch/cloud-media-service/` owns the specification and ticket dependency graph.
- Playback and Trigger firmware consume the finalized manifest and authenticated HTTP contracts but are not implemented by these backend tickets.
