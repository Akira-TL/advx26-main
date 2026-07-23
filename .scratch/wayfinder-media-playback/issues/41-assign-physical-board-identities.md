# Assign the two connected T5AI boards to demo roles

Type: grilling
Status: resolved
Blocked by: 30

## Question

Which connected USB Dual Serial identity is the validated Playback Board and which is the Trigger Board?

## Answer

Assign the two managed T5AI boards by stable USB serial identity:

- `5AAE167197` is the **Playback Board**;
- `5AAE167460` is the **Trigger Board**.

Use `/dev/serial/by-id/usb-1a86_USB_Dual_Serial_<identity>-if00` for downloading and the matching `if02` path for logs. Do not rely on mutable `/dev/ttyACM*` numbering.

The additional physically connected board remains an **Unmanaged Board**. It is not a role candidate and must not be flashed, opened over serial, or otherwise operated on without explicit user authorization.
