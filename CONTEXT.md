# Domain glossary

## Trigger Board

One Tuya T5AI board with the same display hardware as the Playback Board. It reads NFC, resolves or constructs the information for a Playback Session, and remains the authoritative controller for that session over the Board Link.

Avoid: sender device, remote board, controller board.

## Playback Board

The Tuya T5AI board that receives a cloud content reference, downloads cloud-hosted video and audio, renders video on its attached display, and sends audio to the Bluetooth Speaker.

Avoid: receiver device, display board, player board when the distinction matters.

## Board Link

The bidirectional Bluetooth Low Energy connection between the Trigger Board and the Playback Board. It carries Playback Session setup data, control commands, acknowledgements, progress, state, errors, and interaction results. It does not carry the media payload itself.

Avoid: audio Bluetooth, speaker Bluetooth.

## Speaker Link

The Bluetooth audio connection between the Playback Board and the Bluetooth Speaker. It carries decoded audio output and is independent of the Board Link.

Avoid: board Bluetooth, BLE link.

## Cloud Media Service

The backend service that accepts, stores, describes, and distributes the media needed by the Playback Board. For the competition prototype it serves video and audio without mandatory authentication and does not require or render STL models.

## Media Package

A cloud-side association of one playable video asset, one playable audio asset of equal duration, and the metadata needed by the Playback Board to retrieve and present them as one experience. A competition Media Package is at most 30 seconds long.

## Playback Session

The bounded lifecycle started by one NFC-triggered content selection. It includes content resolution, loading, playback control, progress reporting, interaction results, completion, interruption, or failure. A newer accepted Playback Session replaces the current one immediately.

## Playback Command

A Trigger Board request that changes a Playback Session, such as loading content, starting, pausing, seeking, stopping, or replacing the session. The exact command vocabulary remains a protocol decision.

## Playback Report

A Playback Board message that reports progress, current state, errors, acknowledgement, completion, or interaction results to the Trigger Board.
