# Make the Trigger Board the playback authority

Type: grilling
Status: resolved
Blocked by: 01, 02

## Question

Which board owns the playback session and interaction flow?

## Answer

The Trigger Board reads NFC, resolves or constructs the information needed for a playback session, and sends it to the Playback Board over the Board Link. It remains the authoritative controller for that session.

The Board Link therefore supports more than initial content delivery. The Trigger Board can control play, pause, seek/progress, stop, and content replacement. The Playback Board reports acknowledgements, current progress, playback state, completion, and errors back to the Trigger Board.

All user interaction remains local to the Trigger Board. The Playback Board is an output-only executor and never reports touch or business-interaction results.
