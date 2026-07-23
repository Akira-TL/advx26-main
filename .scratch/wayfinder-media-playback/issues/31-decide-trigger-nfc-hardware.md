# Decide the Trigger Board NFC hardware contract

Type: grilling
Status: resolved
Blocked by:

## Question

Which NFC reader/module is attached to the Trigger Board, and what hardware responsibility belongs to this repository?

## Answer

The Trigger Board uses a PN532 reader and only needs the logical result of reading an NDEF URL.

PN532 electrical interface selection, wiring, pin assignment, board bring-up, and low-level driver integration are explicitly outside this workstream. Another hardware integration component owns those details and must expose a narrow application-facing seam to Trigger firmware, such as:

```c
void on_ndef_url(const char *url, size_t length);
```

The Trigger application validates and consumes the URL after that seam. It does not parse arbitrary NFC record types, write tags, configure PN532 pins, or contain board-specific PN532 transport code in the shared playback implementation.
