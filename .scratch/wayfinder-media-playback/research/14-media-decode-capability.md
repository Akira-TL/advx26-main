# Device-ready media capability research

## Conclusion

The competition device profile is an indexed JPEG frame stream plus an independent MP3 audio asset. WebM and MP4/H.264 are not the Playback Board contract.

The T5-E1-IPEX material confirms H.264 encoding capability, not a reusable local H.264 playback decoder. Inspection of TuyaOpen v1.9.0 found JPEG decode/display paths and MP3 decode support, but no general MP4 demux plus H.264 decode pipeline for cloud-file playback.

## Primary-source evidence

- Tuya T5AI-Board documentation: `https://developer.tuya.com/cn/docs/iot-device-dev/T5-E1-IPEX-development-board?id=Ke9xehig1cabj`.
- The linked T5-E1-IPEX module specification describes a 720p H.264 video encoder rather than a local playback decoder.
- TuyaOpen exposes JPEG metadata/decode APIs in `/home/akira/SDKs/TuyaOpen-v1.9.0/src/tal_image/include/tal_image_jpeg_codec.h`.
- The T5AI multimedia pipeline decodes JPEG frames and submits them to the LCD service through `jpeg_decode_pipeline.c`.
- The common audio player integrates MP3 decoding through `src/audio_player/src/decoder/decoder_mp3.c`.
- A2DP Source accepts application-provided PCM at supported sample rates through `bk_dm_a2dp.h`.
- The misleading `h264_jdec_pipeline_*` functions open an H.264 encoding plus JPEG display pipeline; they are not H.264 decoding entry points.
- No general MP4 demuxer or cloud-file H.264 decoder was found in the inspected TuyaOpen v1.9.0 application path.

## Selected competition profile

### Video

- Resource: `video.mjpg` containing complete baseline JPEG frames concatenated in presentation order.
- Index: `video.idx` containing timestamp, byte offset, and byte length for every frame.
- Logical dimensions: 480x320 landscape.
- Baseline rate: 10 fps.
- Stretch target: 15 fps only if the hardware prototype remains stable.
- Initial JPEG quality target: approximately 65, tuned by the prototype.
- Maximum duration: 30 seconds.

### Audio

- Resource: `audio.mp3`.
- Encoding: MP3 CBR.
- Initial bitrate: 128 kbps.
- Sample rate: 44.1 kHz.
- Channel count: preserve mono or stereo, capped at two channels.
- Decode output: PCM S16LE supplied to A2DP Source.

## Synchronization implication

Decoded PCM consumption is the media clock. JPEG frames are scheduled by their presentation timestamps. A late video frame is dropped rather than stalling audio. A fixed speaker-output latency offset is calibrated during the hardware prototype.

## Required prototype gate

Run a 30-second device test with JPEG decode, MP3 decode, A2DP output, BLE control, and progress reports active together. Record frame drops, JPEG decode time, MP3 decode load, memory high-water mark, PCM underruns, Bluetooth errors, and visible synchronization error.
