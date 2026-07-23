# Decide the fixed Bluetooth Speaker identity

Type: grilling
Status: resolved
Blocked by: 09

## Question

How is the fixed Bluetooth Speaker paired and identified for the first prototype and competition demo?

## Answer

Use one fixed A2DP Sink. Perform one explicit manual pairing during hardware setup, persist the resulting pairing/bond information, and reconnect automatically on later boots.

The exact speaker model, advertised name, and Bluetooth address are deployment inputs rather than architectural decisions. Record them in the local demo configuration or pairing record when the physical speaker is selected; do not build a general speaker picker into either board UI.

If the stored target is unavailable, Playback Board reports a speaker-link error over the Board Link and keeps retrying in the background. It must not silently send audio to another discovered device.
