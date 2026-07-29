# Lingguang H5 Frontend

React + TypeScript H5 frontend for the SoundPola Lingguang pages.

## Stack

- React 18 + TypeScript
- Vite 7
- Tailwind CSS
- Radix UI
- Ant Design Mobile
- Howler.js for audio playback
- Native `video` element for visual playback

## Pages

- `/` - download landing page
- `/preview?id={content_id}` - content preview page
- `/c/{content_id}` - content preview page route shape, when the hosting layer falls back to `index.html`

The preview page also extracts a 32-character content id from hash or the current URL.

## Preview Data

The H5 page does not consume the backend-rendered HTML from `/c/{content_id}`. It fetches token metadata and builds media URLs directly:

- metadata: `GET {VITE_API_BASE}/api/v1/contents/{content_id}/token-metadata`
- video: `{VITE_API_BASE}/preview/{content_id}/video`
- audio: `{VITE_API_BASE}/preview/{content_id}/audio`

## Development

```bash
npm install
npm run dev
```

## Build

```bash
npm run build
```

The build output is written to `dist/` as static HTML/CSS/JS assets.
