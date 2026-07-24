# Decide demo Wi-Fi provisioning

Type: grilling
Status: resolved
Blocked by: 01

## Question

How do both T5AI boards obtain network credentials, and how do they reach the Cloud Media Service in the competition demo?

## Answer

Use compile-time demo configuration for both boards. Trigger and Playback firmware receive the same fixed SSID/password and public Cloud Media Service base URL through local build-time configuration excluded from Git. Trigger additionally receives only the fixed Trigger Token, while Playback receives only the fixed Playback Token.

The first demo does not implement BLE provisioning, Tuya cloud provisioning, captive portals, or an on-device credential editor. Both boards join the available competition network independently and access the backend through its public domain.

The checked-in code must contain placeholders and fail clearly when required local Wi-Fi, backend URL, or role-specific device-token configuration is absent. Real credentials must not be committed even though production-grade secret management is out of scope.
