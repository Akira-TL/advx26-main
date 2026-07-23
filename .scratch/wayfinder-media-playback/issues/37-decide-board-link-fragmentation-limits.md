# Decide Board Link payload limits and fragmentation

Type: grilling
Status: resolved
Blocked by: 15, 36

## Question

What maximum size may a LOAD_SESSION descriptor have, how are payloads larger than the negotiated BLE ATT payload fragmented and reassembled, and which URL/field limits should both boards enforce?

## Answer

A reconstructed Board Link JSON message may be at most 4096 bytes. Any individual URL field may be at most 1024 UTF-8 bytes. Both boards reject messages that exceed either limit before allocating the full destination object.

The sender derives the fragment body capacity from the negotiated ATT payload minus the fixed protocol envelope. It sends fragments in ascending order with one shared message ID, zero-based fragment index, total fragment count, and fragment payload length.

Only one fragmented Command message may be assembled at a time because the protocol already permits only one command in flight. Playback Board discards and NACKs the partial message when:

- a fragment is malformed or exceeds the negotiated capacity;
- indices or counts conflict;
- the reconstructed length would exceed 4096 bytes;
- all fragments are not received within five seconds.

A new message ID replaces an incomplete older assembly only after the older message has been rejected. JSON parsing and schema validation occur only after full reassembly.
