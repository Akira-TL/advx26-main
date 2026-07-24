# Implement Board Link body and fragment codec

Type: task
Status: resolved
Blocked by: 01

## Goal

Implement the transport-independent codec for the frozen Board Link v1 binary fragment envelope and JSON body schema.

## Functional scope

- encode and decode the fixed 16-byte little-endian fragment header;
- enforce protocol major, message kind, flags, message ID, fragment index/count, payload length, and reserved-field rules;
- cap reconstructed JSON at 4096 bytes and individual URLs at 1024 bytes;
- reassemble by direction and message ID, reject conflicting duplicates, and expire incomplete assemblies after five seconds;
- parse strict versioned JSON into `playback_command_t`;
- serialize `playback_report_t` into the common body envelope;
- ignore unknown optional fields within the same major version while rejecting missing or mistyped required fields;
- map malformed bodies to stable protocol NACK reasons.

## Module seam

The codec accepts byte buffers and returns domain commands or serialized report fragments. It must not know about GATT connections, Playback policy, media state, or notification scheduling.

## Completion criteria

- Command and Report use the frozen UUID-independent wire schema from Wayfinder issues 48 and 52;
- fragmented and unfragmented bodies follow the same reconstruction path;
- no application checksum is introduced beyond existing framing and media-integrity rules.

## Out of scope

BLE advertising, characteristic registration, command execution, tests, build, flash, and hardware debugging.

## Answer

Implemented the fixed 16-byte little-endian fragment envelope, bounded fragmentation and direction-scoped reassembly, duplicate/conflict handling, five-second expiry, strict UTF-8/versioned JSON command parsing, normalized Media Package validation, report serialization, and stable NACK mapping. The implementation is recorded in playback commits `b24aa80` and `48049a2`.
