# Decide Board Link discovery and reconnection

Type: grilling
Status: resolved
Blocked by: 11, 13

## Question

How should the two fixed boards discover, connect, and reconnect over BLE for the competition demo?

## Answer

Playback Board advertises a dedicated Board Link Service UUID and an auxiliary human-readable device name. Trigger Board scans for the Service UUID, connects automatically at boot, discovers the required characteristics, enables notifications, and continuously retries after disconnection without user intervention.

Do not depend on a random BLE address, manually entered address, or a user-facing device picker. The first demo does not require encrypted pairing or bonding for the Board Link.

If more than one matching Playback Board is visible, Trigger Board deterministically chooses the strongest candidate only when no prior peer identity is stored; after the first successful connection it may remember that peer as a preference while continuing to validate the Service UUID and protocol version.
