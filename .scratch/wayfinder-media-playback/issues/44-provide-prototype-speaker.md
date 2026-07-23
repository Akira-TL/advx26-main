# Provide the fixed speaker for hardware prototypes

Type: task
Status: open
Blocked by: 33, 42

## Question

Supply one physical Bluetooth A2DP Sink when Playback enters the hardware-debugging stage. This is not a blocker for Playback feature planning or code decomposition.

Do not start pairing, discovery, address capture, or speaker-specific configuration during the current functional implementation phase.

The task is complete when:

- the speaker is physically available and charged/powered;
- it can be placed in pairing mode;
- its advertised Bluetooth name is recorded;
- its Bluetooth address is recorded when discoverable;
- it is explicitly approved as the fixed prototype target.

Do not use another discovered Bluetooth device as a substitute without user approval.
