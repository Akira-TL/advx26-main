# Decide Board Link command retry and idempotency

Type: grilling
Status: resolved
Blocked by: 11, 36

## Question

How many commands may be in flight, what timeout and retry policy should Trigger use, and how must Playback recognize duplicate sequence IDs so retried LOAD/PLAY/PAUSE/SEEK/STOP commands do not execute twice?

## Answer

Use a serialized command channel with at most one unacknowledged Playback Command in flight.

Each command carries a monotonically increasing unsigned 32-bit `sequence_id`. Trigger Board starts a one-second acknowledgement timer after the final fragment is written. If no correlated ACK or NACK arrives, Trigger retries the same command with the same `sequence_id` up to three times. Exhausting the retries moves the Board Link to an error/reconnect path instead of issuing later commands out of order.

Playback Board keeps a bounded cache of the sixteen most recent command results for the current protocol connection epoch:

- a duplicate `sequence_id` with the same command identity returns the cached ACK/NACK and does not execute again;
- a duplicate `sequence_id` carrying different command content is rejected as a protocol error;
- a new sequence executes once, commits its result to the cache, then reports the correlated result.

`LOAD_SESSION`, `PLAY`, `PAUSE`, `SEEK_MS`, and `STOP` must therefore be safe under retransmission. Progress and state notifications are reports rather than commands and do not participate in this retry window.
