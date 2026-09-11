# Pulse Player FFmpeg

Native FFmpeg bindings for Node.js and Electron, built with
[Node-API](https://nodejs.org/api/n-api.html).

`@freetalk-team/pulse-player-ffmpeg` provides a native media-processing
interface for applications that need to read and modify metadata, extract
album artwork, generate thumbnails, and record media streams.

The project is designed to work with both regular Node.js applications and
Electron applications without requiring a rebuild for a specific Electron
version.

## Features

- 🎵 Read media metadata
- ✏️ Update media metadata
- 🖼️ Extract embedded cover artwork
- 🎞️ Generate video thumbnails
- 📐 Scale thumbnails proportionally to a maximum width
- 🎙️ Record media streams
- 📊 Query recording status
- 🔌 Node-API based native addon
- 🖥️ Node.js and Electron compatible
- 📦 Prebuilt binaries for supported platforms
- ⚡ FFmpeg integrated directly into the native addon
- 🦀 dav1d support for AV1 decoding

## Installation

Install the main package from npm:

```bash
npm install @freetalk-team/pulse-player-ffmpeg
```

The package uses platform-specific optional dependencies to install the
appropriate native binary for the current operating system and architecture.

## Supported Platforms

| **Platform** | **Architecture** |
| ------------ | ---------------- |
| Linux        | x64              |
| Linux        | arm64            |
| Windows      | x64              |
| Windows      | arm64            |

Additional platforms may be supported in future releases.


## Usage

### Import

```javascript
import {
    parseMeta,
    updateMeta,
    generateThumbnail,
    startRecording,
    stopRecording,
    getRecordingStatus
} from '@freetalk-team/pulse-player-ffmpeg';
```

CommonJS is also supported:

```javascript
const {
    parseMeta,
    updateMeta,
    generateThumbnail,
    startRecording,
    stopRecording,
    getRecordingStatus
} = require('@freetalk-team/pulse-player-ffmpeg');
```

---

### Metadata

#### Reading metadata

Use `parseMeta()` to read metadata from a media file:

```javascript
const metadata = parseMeta('/music/example.mp3');

console.log(metadata.title);
console.log(metadata.artist);
console.log(metadata.album);
console.log(metadata.genre);
```

The addon uses FFmpeg's format and metadata support, allowing it to work with
different media containers and codecs.

Depending on the media format, a metadata field may contain either a single
value or multiple values.

```javascript
await updateMeta('/music/example.mp3', {
    title: 'Example Song',
    artist: 'Example Artist',
    album: 'Example Album',
    genre: 'Electronic'
});
```

The exact metadata fields supported depend on the media container and its
metadata format.

---

### Cover artwork

Embedded cover artwork can be read and written as part of media metadata.

For example:
```javascript
const metadata = parseMeta('/music/example.mp3');

if (metadata.cover) {
    console.log(metadata.cover);
}
```

Cover artwork is represented by the TypeScript definitions included with the
package.

---

### Thumbnails

`generateThumbnail()` extracts a frame from a video and produces an image
thumbnail.

```javascript
const thumbnail = generateThumbnail('/video/example.mp4', cover.jpg, 15, 640);
```

When the source image is wider than the requested maximum width, it is scaled
proportionally while preserving its aspect ratio.

This makes it suitable for generating thumbnails for media libraries,
video previews, album artwork, and similar applications.

---

### Recording

The addon provides an API for recording media streams.

### Start recording

```javascript
const recording = startRecording('https://example.com/stream', 'stream.mp3', { title: 'Best radio' });
```

> Note: Transcoding is not supported. The input stream format must match the output file format.

#### Get recording status

```javascript
const status = getRecordingStatus(recording.id);

console.log(status);
```

#### Stop recording

```javascript
stopRecording(recording.id);
```


### TypeScript

TypeScript declarations are included with the main package.

```typescript
import type { Metadata } from '@freetalk-team/pulse-player-ffmpeg';

const metadata: Metadata = {
    title: 'Example Song',
    artist: ['Example Artist'],
    album: 'Example Album',
    genre: ['Electronic', 'Ambient']
};
```

The declarations are available in:

```
packages/pulse-player-ffmpeg/index.d.ts
```


## Licensing

Copyright © 2026 Freetalk Team

Pulse Player FFmpeg is free and open-source software.

The source code of this project is distributed under the terms specified in
[LICENSE](./LICENSE).

Third-party libraries bundled with the native binaries are distributed under
their respective licenses. In particular, **FFmpeg** and **dav1d** have their own
copyright and licensing terms.

See the license files included with the corresponding third-party sources for
complete information.


## Issues

If you encounter a problem, please open an issue and include:

* Operating system
* CPU architecture
* Node.js version
* Electron version, if applicable
* Package version
* A minimal reproduction, if possible
* Relevant error output

This information makes it much easier to diagnose native addon problems.


## Funding

Pulse Player FFmpeg is developed and maintained as free and open-source
software by the Freetalk Team.

If this project is useful to you, consider supporting its development.

Funding helps us continue working on:

* Cross-platform native support
* FFmpeg integration
* Node.js and Electron compatibility
* Media metadata support
* Thumbnail generation
* Recording functionality
* Testing and CI
* Documentation
* Bug fixes and maintenance

You can support the project through the funding options available on the
GitHub repository.


## Related Projects

**Pulse Player FFmpeg** is developed as part of the [Pulse Player](https://github.com/freetalk-team/pulse-player) project.

Pulse Player is a free and open-source desktop media player built with
Electron and Svelte.


## Author

### Freetalk Team

[www.sipme.io](https://www.sipme.io/)

---

## License

See [LICENSE](./LICENSE) for the license covering this project.

Third-party components retain their respective licenses.