# Decide the Board Link command and report model

Type: grilling
Status: resolved
Blocked by: 05

## Question

Which Playback Commands, acknowledgements, progress reports, state transitions, and interaction-result messages must the first Board Link protocol support?

## Answer

The first Board Link protocol supports these Trigger Board commands:

- `LOAD_SESSION`: install or replace the current Playback Session using the resolved session descriptor.
- `PLAY`: begin or resume playback.
- `PAUSE`: pause video and audio at the current position.
- `SEEK_MS`: move both video and audio to a requested millisecond position.
- `STOP`: stop playback and return to the loaded/idle presentation state.

A newly accepted `LOAD_SESSION` immediately replaces the active session. Every command carries a command identifier and receives an immediate `ACK` or `NACK` correlated to that identifier.

The Playback Board reports:

- state changes immediately;
- playback progress every 500 ms while playing;
- completion immediately;
- recoverable and terminal errors immediately.

The Board Link does not carry user-interaction results. All user interaction occurs on the Trigger Board and is translated locally into Playback Commands.
