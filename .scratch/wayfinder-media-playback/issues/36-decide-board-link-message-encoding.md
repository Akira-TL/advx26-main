# Decide Board Link message encoding and characteristics

Type: grilling
Status: resolved
Blocked by: 11, 35

## Question

Should Board Link use JSON, CBOR, or fixed binary payloads, and how many GATT characteristics should carry Trigger-to-Playback commands and Playback-to-Trigger reports?

## Answer

Use one custom Board Link GATT service with two application characteristics:

- **Command characteristic**: Trigger Board writes with response; Playback Board receives session setup and playback commands.
- **Report characteristic**: Playback Board notifies; Trigger Board receives command results, state, progress, completion, and errors.

Each ATT value is a protocol fragment composed of a compact fixed-width binary envelope followed by a UTF-8 JSON fragment. The envelope identifies the protocol version, message kind, message ID, fragment index/count, and fragment payload length. JSON remains the application message format because it is easy to inspect in serial logs and quick to evolve during the competition.

The application does not expose one characteristic per command and does not use CBOR or fully fixed binary domain messages in the MVP. The exact UUID values and byte offsets belong in the protocol specification, but both firmware applications consume one shared definition from `firmware/shared/`.
