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

## Cloud Media Service

The backend service that accepts, stores, describes, and distributes the media needed by the Playback Board. For the competition prototype it serves video and audio without mandatory authentication and does not require or render STL models.

## Media Package

A cloud-side association of one playable video asset, one playable audio asset of equal duration, and the metadata needed by the Playback Board to retrieve and present them as one experience. A competition Media Package is at most 30 seconds long.

## Compact Content URL

The short HTTP URL stored on an NFC tag. The Trigger Board resolves it through the Cloud Media Service to obtain detailed content, playback, and Trigger Control Panel metadata.

## Playback Session

The bounded lifecycle started by one NFC-triggered content selection. It includes content resolution, loading, playback control, progress reporting, completion, interruption, or failure. A newer accepted Playback Session replaces the current one immediately.

## Playback Command

A Trigger Board request that changes a Playback Session. The first protocol vocabulary is `LOAD_SESSION`, `PLAY`, `PAUSE`, `SEEK_MS`, and `STOP`.

## Playback Report

A Playback Board message that acknowledges or rejects a command, reports current playback state and progress, signals completion, or reports an error to the Trigger Board. Progress is reported every 500 ms while playing.

## Trigger Control Panel

The minimal user interface shown on the Trigger Board after it resolves a Compact Content URL. It presents playback information, progress, playback controls, and visible feedback for local interaction. It intentionally hides routine BLE, speaker, and loading diagnostics from the normal experience.
