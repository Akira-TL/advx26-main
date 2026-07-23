# Decide the two-board firmware repository layout

Type: grilling
Status: resolved
Blocked by: 01

## Question

Should Trigger Board firmware and Playback Board firmware live as two applications in this repository, should Trigger firmware remain in another existing project, or should each board use a separate repository/submodule?

## Answer

Keep both T5AI firmware applications in this repository and share protocol/domain code between them. The target layout is:

```text
firmware/
├── shared/       # Board Link protocol, shared value objects, validation helpers
├── playback/     # Playback Board application
└── trigger/      # Trigger Board application
```

The existing canonical `firmware/` application is the Playback Board baseline and must be migrated rather than duplicated. Trigger-only NFC/control UI code stays out of Playback firmware, while transport framing and shared session models must not be copied into each application.

Repository restructuring is an implementation task after the Wayfinder map becomes a specification; this decision does not move files during planning.
