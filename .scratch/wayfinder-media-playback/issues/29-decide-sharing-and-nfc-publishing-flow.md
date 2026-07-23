# Decide the Shared Sound submission and NFC publishing flow

Type: grilling
Status: resolved
Blocked by: 24, 25

## Question

For the competition MVP, how does a user submit or record a Shared Sound, and which device writes the resulting Compact Content URL to NFC: a web/mobile interface, a desktop operator tool, a dedicated board, or pre-written demo tags?

## Answer

Use a phone-facing sharing interface. The user records a sound or selects an existing audio file, submits it to the Cloud Media Service, waits for the Media Package to become ready, and then writes the returned Compact Content URL to an NFC tag with the same phone.

The normal product flow does not require the Trigger Board to record, upload, or write NFC. For competition reliability, the team may also prepare known-good NFC tags in advance, but that is a fallback demonstration procedure rather than the canonical product contract.

The exact mobile implementation technology remains an implementation decision. It must support audio capture/upload, readiness feedback, and NFC URL writing on the selected demo phone.
