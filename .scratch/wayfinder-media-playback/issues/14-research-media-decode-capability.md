# Research device-ready video and audio formats

Type: research
Status: resolved
Blocked by:

## Question

Which video and audio representation should the Cloud Media Service prepare for reliable Playback Board decoding and Bluetooth speaker output?

## Answer

Use the bounded device profile `t5ai-jpeg-mp3-v1`:

- video is a sequence of independently decodable baseline JPEG frames packed into `video.mjpg`;
- `video.idx` records presentation timestamp, byte offset, and byte length for every frame;
- output is 480x320 landscape at 10 fps, with 15 fps only as a prototype stretch target;
- audio is a separate `audio.mp3` asset using 128 kbps CBR, 44.1 kHz, and at most two channels;
- Playback decodes MP3 to PCM S16LE and supplies PCM to A2DP Source;
- decoded PCM consumption is the synchronization clock and late video frames are dropped.

MP4/H.264 is not the device contract. Official T5-E1-IPEX material confirms H.264 encoding rather than a reusable local playback decoder, and the inspected TuyaOpen v1.9.0 application path does not provide general MP4 demux plus H.264 decoding.

Final JPEG quality, 10/15 fps viability, memory use, speaker latency, and concurrent BLE/A2DP stability remain subject to the hardware prototype.

Research asset: [Device-ready media capability research](../research/14-media-decode-capability.md).
