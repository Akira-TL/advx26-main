# Decide source audio acceptance and normalization

Type: grilling
Status: resolved
Blocked by: 08, 24, 28, 47

## Question

Which phone-produced audio formats may the Cloud Media Service accept, how are they identified, what upload limit applies, and what exact normalization produces the `audio.mp3` asset?

Decide whether the backend accepts common native phone formats and inspects them with FFprobe, accepts only WAV/MP3, or requires the Sharing Interface to upload an already normalized MP3.

## Answer

The Cloud Media Service accepts common phone-produced audio containers and codecs instead of requiring the Sharing Interface to pre-transcode them. The initial accepted input set is WAV, MP3, M4A/AAC, Ogg/Opus, and WebM audio.

The backend must inspect the uploaded bytes with FFprobe. File extensions, client MIME types, and filenames are advisory only. A source is rejected when FFprobe cannot parse it, when it contains no decodable audio stream, when the selected audio stream is encrypted, or when decoding fails.

The upload contract is bounded as follows:

- maximum uploaded file size: 50 MiB;
- select the first valid audio stream;
- maximum normalized duration: 30 seconds;
- for longer sources, retain the first 30 seconds;
- for shorter sources, retain the complete duration without padding;
- accept any source sample rate supported by FFmpeg;
- accept any source channel layout supported by FFmpeg, then downmix to at most two channels;
- discard embedded metadata, cover art, chapters, and non-audio streams.

The normalized device asset is:

- filename: `audio.mp3`;
- codec: MP3;
- bitrate mode: 128 kbps CBR;
- sample rate: 44.1 kHz;
- sample format after board-side decode: signed 16-bit PCM;
- channels: preserve mono sources as mono and use stereo for sources with two or more channels;
- peak limiting target: approximately -1 dBFS to prevent clipping after resampling or downmixing.

Normalization runs inside the private staging revision. The backend measures the decoded normalized duration and uses that value as the authoritative Media Package duration before trimming or looping the Preset Video. A revision cannot become `READY` unless the generated MP3 can be probed and decoded successfully and its generated `audio.idx` passes validation.
