# Decide source audio acceptance and normalization

Type: grilling
Status: resolved
Blocked by: 08, 24, 28, 47, 55

## Question

Which phone-produced audio formats may the Cloud Media Service accept, how are they identified, what upload limit applies, and what exact normalization drives both `audio.mp3` and cloud visualization rendering?

## Answer

The Cloud Media Service accepts common phone-produced audio containers and codecs instead of requiring the Sharing Interface to pre-transcode them. The initial accepted input set is WAV, MP3, M4A/AAC, Ogg/Opus, and WebM audio.

The backend inspects uploaded bytes with FFprobe. File extensions, client MIME types, and filenames are advisory only. A source is rejected when FFprobe cannot parse it, when it contains no decodable audio stream, when the selected audio stream is encrypted, or when decoding and bounded repair both fail. The authenticated owner's original upload is permanently retained even after successful processing.

The upload contract is bounded as follows:

- maximum uploaded file size: 50 MiB;
- select the first valid audio stream;
- maximum normalized duration: 30 seconds;
- retain the first 30 seconds of longer sources;
- retain the complete duration of shorter sources without padding;
- accept source sample rates and channel layouts supported by FFmpeg;
- downmix to at most two channels;
- discard embedded metadata, cover art, chapters, and non-audio streams from generated assets.

The normalized device asset is:

- filename: `audio.mp3`;
- codec: MP3;
- bitrate mode: 128 kbps CBR;
- sample rate: 44.1 kHz;
- board-side decoded sample format: signed 16-bit PCM;
- preserve mono sources as mono and use stereo for sources with two or more channels;
- peak limiting target approximately -1 dBFS.

The normalized decoded timeline is authoritative for `duration_ms` and is also the input timeline for the headless `Sound-Visualization-Kaleidoscope-effect` render. A content item cannot become READY unless the MP3 can be probed and decoded, the generated visual aligns to that duration, and `audio.idx` passes complete validation.
