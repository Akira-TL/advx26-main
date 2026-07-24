# Decide the device media manifest contract

Type: grilling
Status: resolved
Blocked by: 10, 14, 54

## Question

What normalized document must the Cloud Media Service expose, and what subset must the Trigger Board send to the Playback Board?

## Answer

A READY Compact Content URL returns one immutable Trigger-facing JSON document after validating the fixed Trigger Token:

```json
{
  "schema_version": 1,
  "content_id": "immutable-content-id",
  "state": "READY",
  "duration_ms": 12000,
  "trigger": {
    "display_label": "声音碎片 #1234",
    "autoplay": true,
    "end_behavior": "HOLD_LAST_FRAME",
    "controls": ["PLAY", "PAUSE", "SEEK", "STOP", "REPLAY"]
  },
  "playback": {
    "profile": "t5ai-h264-mp3-v1",
    "video": {
      "url": "https://server/api/v1/contents/.../assets/video",
      "format": "MP4_H264",
      "codec": "H264",
      "codec_profile": "BASELINE",
      "pixel_format": "YUV420P",
      "width": 480,
      "height": 320,
      "fps": 10,
      "max_keyframe_interval_ms": 1000,
      "byte_length": 123456,
      "sha256": "lowercase-hex",
      "etag": "strong-etag"
    },
    "audio": {
      "url": "https://server/api/v1/contents/.../assets/audio",
      "index_url": "https://server/api/v1/contents/.../assets/audio-index",
      "format": "MP3_CBR",
      "bitrate_kbps": 128,
      "sample_rate": 44100,
      "channels": 2,
      "byte_length": 12345,
      "sha256": "lowercase-hex",
      "etag": "strong-etag"
    }
  }
}
```

One upload creates one immutable `content_id`; the competition contract does not add a separate public revision number. Reprocessing or republishing creates a new content item and NFC URL rather than mutating READY bytes.

The Trigger Board validates `schema_version`, `state`, duration, and profile, creates a local `session_id`, then sends the normalized `playback` descriptor plus `content_id`, `duration_ms`, and `session_id` over the Board Link. The Trigger Token is never forwarded.

Asset URLs are absolute stable backend endpoints, not anonymous object paths or signed URLs. Playback supplies its own fixed Playback Token on every descriptor, `GET`, `HEAD`, and Range request. The video descriptor has no separate video index; MP4 sample tables provide video timing and ranges. `audio.idx` remains mandatory for precise MP3 seeking.

User ownership and source-object metadata are intentionally absent from the device document. They remain available only through User Token-protected management APIs.
