# Firmware boundaries

SoundPola uses separate ownership boundaries for the two T5AI roles.

```text
firmware/
├── playback/   # Git submodule: implemented Playback Board firmware
└── trigger/    # reserved Trigger Board location
```

## Playback

`playback/` is the `advx26-playback` Git submodule. Commit Playback Board source changes inside that repository first, then commit the updated gitlink in the parent repository.

## Trigger

`trigger/` is intentionally only a reservation. When Trigger development begins, replace the reservation with a dedicated Git repository/submodule rather than adding Trigger implementation to the Playback repository.

Shared cross-project contracts remain in the parent repository until a dedicated shared protocol package is introduced.
