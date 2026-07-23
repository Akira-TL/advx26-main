# Set replacement behavior and start latency

Type: grilling
Status: resolved
Blocked by:

## Question

How should new content replace current playback, and how quickly should playback start?

## Answer

A newly accepted playback session immediately interrupts and replaces the current session. There is no queue in the first demo.

The target is to begin playback approximately 3–5 seconds after the Playback Board receives a valid session request. Short prebuffering is acceptable; a fully general low-latency streaming protocol is not required.
