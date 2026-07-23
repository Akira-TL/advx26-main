# Decide Playback Board interaction semantics

Type: grilling
Status: resolved
Blocked by: 05

## Question

What user interactions occur on the Playback Board, and what semantic result must be returned to the Trigger Board? Decide whether results are action identifiers, option selections, touch coordinates, or another domain event.

## Answer

The Playback Board has no user interaction in the first demo. Its screen is a media output surface only; it does not expose playback controls, clickable overlays, option buttons, or business interactions.

After the Trigger Board resolves the NFC URL and receives the content description, it displays the control interface on its own screen. That interface contains the playback information, current progress, and visible feedback for the user's controls or content-specific interaction.

The Trigger Board translates local user actions directly into Board Link Playback Commands. No touch coordinates, option selections, or semantic interaction-result messages are sent from the Playback Board.

The Trigger Board interface remains intentionally minimal: it does not foreground speaker status, BLE diagnostics, loading telemetry, or other engineering details during normal use.
