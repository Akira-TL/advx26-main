# Decide the Trigger Control Panel content contract

Type: grilling
Status: resolved
Blocked by: 10, 12

## Question

Which playback metadata and interaction definitions must the Compact Content URL response provide so the Trigger Board can render its control panel?

## Answer

The first Trigger Control Panel is a standard playback interface, not a content-authoring interface. The Compact Content URL response provides enough information to show:

- a generated content label or identifier;
- total duration;
- current playback state and progress;
- play, pause, seek, stop, and replay controls;
- visible feedback for button presses, loading, completion, and errors.

Users do not submit a required title or description, and the MVP does not define per-content custom buttons, choices, or business interactions. Routine BLE, speaker, and transport diagnostics remain hidden from the normal user experience.

The exact generated label shown for an untitled sound is a separate presentation decision.
