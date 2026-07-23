# Decide Board Link JSON body schema

Type: grilling
Status: resolved
Blocked by: 11, 36, 38, 39, 48, 51

## Question

What common JSON envelope and exact command/report vocabularies are carried inside the binary Board Link fragments?

Decide whether every body uses one versioned schema with strict required fields and enumerated message types, or whether each command/report uses an ad hoc object shape. Also decide whether protocol timestamps rely only on media positions and sequence IDs or include unsynchronized wall-clock values.

## Answer

Every reconstructed Board Link JSON body uses one versioned top-level object:

```json
{
  "schema_version": 1,
  "type": "PLAY",
  "session_id": "session-id",
  "sequence_id": 42,
  "payload": {}
}
```

Top-level rules are:

- `schema_version` is the integer `1` for this protocol major;
- `type` is one of the frozen command or report enums below;
- `session_id` is a UTF-8 string for session-scoped messages and JSON `null` for connection-scoped handshake/status messages;
- `sequence_id` is an unsigned 32-bit integer;
- `payload` is always a JSON object, including when empty;
- the complete UTF-8 JSON body remains within the previously selected 4096-byte limit;
- each URL remains within 1024 bytes, and the complete `LOAD_SESSION` body must still fit the overall 4096-byte limit.

Unknown optional payload fields are ignored within the same compatible protocol major. Unknown message types, missing required fields, wrong JSON types, invalid enum values, out-of-range integers, or non-object payloads are rejected with `NACK`. Neither board sends wall-clock timestamps because their clocks are not synchronized. Protocol timing uses `position_ms`, `duration_ms`, `sequence_id`, and connection-local `boot_id` values only.

## Commands

Command `type` values are:

- `HELLO`;
- `GET_STATUS`;
- `LOAD_SESSION`;
- `PLAY`;
- `PAUSE`;
- `SEEK_MS`;
- `STOP`.

`HELLO` is connection-scoped and carries:

```json
{
  "protocol_major": 1,
  "protocol_minor": 0,
  "role": "TRIGGER",
  "boot_id": "boot-id",
  "capabilities": ["BOARD_LINK_V1", "T5AI_JPEG_MP3_V1"],
  "max_message_bytes": 4096
}
```

`GET_STATUS`, `PLAY`, `PAUSE`, and `STOP` use an empty payload. `SEEK_MS` requires one integer `position_ms` bounded to the active session duration.

`LOAD_SESSION` requires `content_id`, `revision`, `duration_ms`, `profile`, and normalized video/audio descriptors. The profile must be `t5ai-jpeg-mp3-v1`. Each asset descriptor contains its absolute immutable URL, exact byte length, SHA-256, and strong ETag. Video additionally carries width, height, frame rate, frame count, stream URL, and index URL. Audio additionally carries sample rate, channel count, bitrate, stream URL, and index URL. Playback rejects the command before downloading when the descriptor exceeds protocol limits, uses an unsupported profile, or contains inconsistent bounds.

## Reports

Report `type` values are:

- `HELLO_ACK`;
- `ACK`;
- `NACK`;
- `STATE`;
- `PROGRESS`;
- `COMPLETED`;
- `ERROR`.

`HELLO_ACK` is connection-scoped and returns Playback's negotiated protocol version, role, `boot_id`, capabilities, and maximum reconstructed message size.

`ACK` and `NACK` use the same `sequence_id` as the command being answered. `ACK.payload` identifies the accepted command type. `NACK.payload` contains a stable rejection `code` and optional diagnostic text. Initial rejection codes are `MALFORMED_MESSAGE`, `UNKNOWN_TYPE`, `INVALID_STATE`, `STALE_SESSION`, `DUPLICATE_SEQUENCE_CONFLICT`, `UNSUPPORTED_PROFILE`, and `PROTOCOL_INCOMPATIBLE`.

`STATE.payload` contains `state`, `position_ms`, `duration_ms`, and retained play/pause intent. `PROGRESS.payload` contains `position_ms` and `duration_ms`. `COMPLETED.payload` contains the final `position_ms` and `duration_ms`. `ERROR.payload` contains the stable Playback error `code`, `retryable`, `position_ms`, and optional diagnostic text.

Asynchronous `STATE`, `PROGRESS`, `COMPLETED`, and `ERROR` reports carry the active `session_id` and the most recently accepted command `sequence_id` for that session. Before any session command is accepted, connection-scoped reports use `session_id: null` and `sequence_id: 0`.
