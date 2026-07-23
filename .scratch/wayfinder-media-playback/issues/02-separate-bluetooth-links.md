# Separate the two Bluetooth links

Type: grilling
Status: resolved
Blocked by:

## Question

How are inter-board control and speaker audio separated?

## Answer

Use two logically independent Bluetooth links on the Playback Board:

- **Board Link**: Bluetooth Low Energy between the Trigger Board and Playback Board. It carries playback-session data, commands, acknowledgements, progress, status, and errors. It never carries the media payload or business interaction results.
- **Speaker Link**: Bluetooth audio from the Playback Board to a fixed Bluetooth Speaker. It carries decoded cloud audio.

The Trigger Board is the playback authority. The Playback Board executes commands and reports state back over the Board Link.
