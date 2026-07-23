# Decide the dual Bluetooth prototype acceptance threshold

Type: grilling
Status: resolved
Blocked by: 41, 42

## Question

What observable threshold proves the BLE Board Link and A2DP Speaker Link can coexist well enough to continue?

## Answer

The prototype passes only when all of the following hold in one continuous run:

- run duration is at least 60 seconds;
- Board Link BLE and Speaker Link A2DP remain connected with zero disconnects;
- Playback emits representative progress reports every 500 ms;
- Trigger completes at least 20 command/ACK exchanges;
- every application ACK arrives within one second, including any successful retry;
- no PCM underrun is reported;
- no Bluetooth controller or profile error is reported;
- the test audio has no obvious interruption, repeated gap, or severe artifact.

Use a low-volume deterministic PCM fixture. Record command latency, report cadence, link events, PCM underruns, controller errors, and audible observations in the prototype answer.
