# Domain glossary

## Trigger Board

One Tuya T5AI board with the same display hardware as the Playback Board. It reads NFC, resolves or constructs the information for a Playback Session, and remains the authoritative controller for that session over the Board Link.

Avoid: sender device, remote board, controller board.

## Playback Board

The Tuya T5AI board that receives a resolved Playback Session descriptor, downloads cloud-hosted video and audio, renders video on its attached display, and sends audio to the Bluetooth Speaker. It is an output-only execution device in the first demo and exposes no user interaction.

Avoid: receiver device, display board, player board when the distinction matters.

## Board Link

The bidirectional Bluetooth Low Energy connection between the Trigger Board and the Playback Board. It carries Playback Session setup data, control commands, acknowledgements, progress, state, and errors. It does not carry media payloads or business interaction results.

Avoid: audio Bluetooth, speaker Bluetooth.

## Speaker Link

The Bluetooth audio connection between the Playback Board and the Bluetooth Speaker. It carries decoded audio output and is independent of the Board Link.

Avoid: board Bluetooth, BLE link.

## Shared Sound

The user-provided source for one experience. A Shared Sound is an audio recording or audio file only. The MVP does not require the user to provide a video, title, description, author profile, or STL model.

## Preset Video

One of a small set of cloud-owned video files prepared in advance for the competition MVP. The Cloud Media Service pairs a Shared Sound with a Preset Video and trims or loops it to the final sound duration. Future generated visuals are a separate asynchronous capability.

## Cloud Media Service

The backend service that accepts a Shared Sound, prepares device-ready media, publishes immutable Compact Content URLs, and distributes the assets needed by the Playback Board. For the competition prototype it operates without mandatory authentication and does not accept or render STL models or user-uploaded video.

## Media Package

A cloud-side association of one Shared Sound, one selected Preset Video, equal-duration indexed JPEG frames and PCM audio, and the metadata needed by the two boards to present them as one experience. A competition Media Package is at most 30 seconds long.

## Compact Content URL

The short HTTP URL stored on an NFC tag. The Trigger Board resolves it through the Cloud Media Service to obtain detailed content, playback, and Trigger Control Panel metadata.

## Playback Session

The bounded lifecycle started by one NFC-triggered content selection. It includes content resolution, loading, playback control, progress reporting, completion, interruption, or failure. A newer accepted Playback Session replaces the current one immediately.

## Playback Command

A Trigger Board request that changes a Playback Session. The first protocol vocabulary is `LOAD_SESSION`, `PLAY`, `PAUSE`, `SEEK_MS`, and `STOP`.

## Playback Report

A Playback Board message that acknowledges or rejects a command, reports current playback state and progress, signals completion, or reports an error to the Trigger Board. Progress is reported every 500 ms while playing.

## Trigger Control Panel

The minimal user interface shown on the Trigger Board after it resolves a Compact Content URL. It presents a generated content label, duration, playback state, progress, standard playback controls, and visible feedback for local interaction. It does not require creator-authored title or description and intentionally hides routine BLE, speaker, and transport diagnostics from the normal experience.

## Sound Fragment Label

The neutral Cloud Media Service-generated identity shown for an untitled Shared Sound. The MVP format is `声音碎片 #XXXX`, where `XXXX` is a short stable identifier derived from the immutable content identifier. Recording or submission time may be shown as secondary information.

## Preset Assignment

The persisted choice of one enabled Preset Video for a new Media Package. The backend derives the initial choice deterministically from the immutable content identifier, then stores the selected preset so later pool changes do not alter an existing published package.

## Sharing Interface

The phone-facing interface used to record or select a Shared Sound, upload it, wait for the Media Package to become ready, and write the resulting Compact Content URL to an NFC tag. It is external to this repository and will be synchronized separately. This repository owns the backend contract it consumes. The Trigger Board is not responsible for recording, uploading, or writing NFC in the normal product flow.

## NFC URL Source

The application-facing seam through which Trigger firmware receives a Compact Content URL decoded from a PN532 NDEF record. This workstream consumes the URL but does not own PN532 wiring, pin assignment, electrical interface selection, or low-level transport/driver integration.

## Firmware Applications

The two T5AI applications maintained in this repository: Trigger firmware and Playback firmware. They use separate application entry points and UI/device modules while sharing the Board Link protocol, validated session value objects, and common test fixtures.

## Demo Network Configuration

Git-ignored build-time values containing the fixed Wi-Fi credentials and public Cloud Media Service base URL used by both boards. Provisioning UX and production secret management are outside the competition MVP.

## Fixed Speaker

The single A2DP Sink selected for the demo. It is manually paired once with Playback Board, stored as the only permitted speaker target, and automatically reconnected on later boots.
