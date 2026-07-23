# Decide Board Link wire constants

Type: grilling
Status: resolved
Blocked by: 36, 37, 38, 39

## Question

Should the first implementation freeze one private 128-bit GATT Service UUID, one Command characteristic UUID, one Report characteristic UUID, and one fixed little-endian fragment-envelope layout?

Decide whether these constants are stable protocol identifiers shared by both firmware applications or generated/configured per build, and whether the envelope includes only framing fields or an additional application checksum.

## Answer

The first Board Link implementation freezes these private 128-bit UUIDs and commits the same constants into Trigger and Playback firmware:

```text
Service UUID: 68d887b5-dd75-4af9-81a3-80af3301c051
Command UUID: c66ff7a5-ec6e-46eb-b679-d837de1c780e
Report UUID:  d1b3806a-61c9-4a7c-a943-d685afcdefbe
```

The Command characteristic uses GATT Write With Response. The Report characteristic uses Notify after Trigger enables its CCCD. UUIDs are protocol identifiers and must not be regenerated per build or per board.

Every ATT value carrying a Board Link fragment begins with this fixed 16-byte little-endian header:

```text
0       magic = 0xA7
1       protocol_major = 1
2       message_kind: 0x01 Command, 0x02 Report
3       flags = 0 for protocol v1
4–7     message_id, uint32 little-endian
8–9     fragment_index, uint16 little-endian, zero-based
10–11   fragment_count, uint16 little-endian, at least 1
12–13   payload_length, uint16 little-endian
14–15   reserved = 0
```

`payload_length` is the number of UTF-8 JSON bytes following the header in the current ATT value. The sender chooses fragment payload size from the negotiated ATT payload capacity minus the 16-byte header.

`message_id` is non-zero and monotonically increases independently in each direction during one Protocol Epoch. A reconnect may restart numbering after HELLO establishes the new epoch. Reassembly remains keyed by direction plus `message_id`.

Receivers reject an invalid magic, unsupported protocol major, unknown message kind, non-zero v1 flags/reserved bytes, zero fragment count, out-of-range fragment index, payload length mismatch, or a reconstructed message above the existing 4096-byte limit.

The envelope adds no application checksum. BLE link protection covers each ATT transfer, JSON length/structure validation covers messages, and media assets use their separate CRC32/SHA-256 integrity model.
