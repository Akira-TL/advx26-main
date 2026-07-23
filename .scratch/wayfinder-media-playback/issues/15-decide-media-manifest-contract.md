# Decide the device media manifest contract

Type: grilling
Status: resolved
Blocked by: 10, 14

## Question

What normalized document must the Cloud Media Service expose, and what subset must the Trigger Board send to the Playback Board?

## Answer

A Compact Content URL returns one immutable Trigger-facing JSON document with these stable sections:

```json
{
  "schema_version": 1,
  "content_id": "immutable-content-id",
  "revision": 1,
  "state": "READY",
  "duration_ms": 12000,
  "trigger": {
    "display_label": "声音碎片 #1234",
    "autoplay": true,
    "end_behavior": "HOLD_LAST_FRAME",
    "controls": ["PLAY", "PAUSE", "SEEK", "STOP", "REPLAY"]
  },
  "playback": {
    "profile": "t5ai-jpeg-mp3-v1",
    "video": {
      "stream_url": "https://server/assets/.../video.mjpg",
      "index_url": "https://server/assets/.../video.idx",
      "format": "JPEG_FRAME_STREAM",
      "width": 480,
      "height": 320,
      "fps": 10,
      "frame_count": 120
    },
    "audio": {
      "url": "https://server/assets/.../audio.mp3",
      "format": "MP3_CBR",
      "bitrate_kbps": 128,
      "sample_rate": 44100,
      "channels": 2
    }
  }
}
```

The Trigger Board validates `schema_version`, `state`, duration, and the named device profile. It creates a local `session_id`, then sends only the normalized `playback` descriptor plus `content_id`, `revision`, `duration_ms`, and `session_id` over the Board Link.

The first contract does not require creator-supplied title, description, author, custom actions, or STL data. Asset URLs are absolute and immutable for the published revision. Exact checksums, byte lengths, JPEG quality metadata, and index binary layout are implementation-specification details rather than new product decisions.
