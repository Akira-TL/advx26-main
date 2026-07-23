# Decide the Preset Video selection policy

Type: grilling
Status: resolved
Blocked by: 24

## Question

When the MVP has several pre-provisioned cloud video files, does every Shared Sound use the same default, does the backend select one deterministically or randomly, or does the user choose one while sharing the sound?

## Answer

The user does not choose a video. The Cloud Media Service deterministically assigns one Preset Video from the configured pool using the immutable sound/content identifier.

The same published Shared Sound revision therefore always resolves to the same Preset Video. Adding or reordering videos must not silently change already-published Media Packages; the selected preset identifier is stored in the package record at publication time.

A simple stable hash modulo the currently enabled preset pool is sufficient when creating a new package. The first implementation may contain only a few manually provisioned videos.
